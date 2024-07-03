//
// Created by yjz on 2022/1/19.
//
#include <regex>
#include "RequestClient.h"
#include "Util/Guid/Guid.h"
#include "Entity/Actor/App.h"
#include "Http/Component/HttpComponent.h"
#include "Util/String/String.h"

namespace http
{
	RequestClient::RequestClient(Component *httpComponent)
		: tcp::TcpClient(0)
	{
		this->mComponent = httpComponent;
	}

	void RequestClient::Do(std::unique_ptr<http::Request> request,
						   std::unique_ptr<http::Response> response, int taskId)
	{
		this->mRequest = std::move(request);
		this->mResponse = std::move(response);
		this->mResponse->Header().Add("t", taskId);
#ifdef ONLY_MAIN_THREAD
		const http::Url &url = this->mRequest->GetUrl();
		this->Connect(url.Host(), url.Port(), 5);
#else
		Asio::Socket &sock = this->mSocket->Get();
		const Asio::Executor &executor = sock.get_executor();
		asio::post(executor, [this]
				   {
			const http::Url & url = this->mRequest->GetUrl();
			this->Connect(url.Host(), url.Port(), this->mRequest->Timeout()); });
#endif
	}

	void RequestClient::OnConnect(bool result, int count)
	{
		if (!result)
		{
			const http::Url &url = this->mRequest->GetUrl();
			CONSOLE_LOG_ERROR("connect {}:{} fail", url.Host(), url.Port());
			this->OnComplete(HttpStatus::INTERNAL_SERVER_ERROR);
			return;
		}
		this->StopTimer();
		if (this->mRequest->Header().KeepAlive())
		{
			this->mSocket->SetOption(tcp::OptionType::KeepAlive, true);
		}
		this->Write(this->mRequest.get());
	}

	void RequestClient::OnSendMessage()
	{
		this->ReadLine(this->mRequest->Timeout());
	}

	void RequestClient::OnSendMessage(const asio::error_code &code)
	{
#ifdef __NET_ERROR_LOG__
		CONSOLE_LOG_ERROR(code.message());
#endif
		this->OnComplete(HttpStatus::INTERNAL_SERVER_ERROR);
	}

	void RequestClient::OnTimeout(tcp::TimeoutFlag flag)
	{
		if (this->mRequest != nullptr)
		{
			const http::Url &url = this->mRequest->GetUrl();
			CONSOLE_LOG_ERROR("{} {} request time out", url.Method(), url.Path())
		}
		switch (flag)
		{
		case tcp::TimeoutFlag::Connect:
			this->OnComplete(HttpStatus::NO_CONTENT);
			break;
		default:
			this->OnComplete(HttpStatus::REQUEST_TIMEOUT);
			break;
		}
	}

	void RequestClient::OnComplete(HttpStatus code)
	{
		if (this->mRequest == nullptr || this->mResponse == nullptr)
		{
			return;
		}
		this->StopTimer();
		this->mSocket->Close();
		if (code != HttpStatus::OK)
		{
			this->mResponse->SetCode(code);
		}
		http::Data *httpData = const_cast<http::Data *>(this->mResponse->GetBody());
		if (httpData != nullptr)
		{
			httpData->OnDecode();
		}
		if (this->mComponent == nullptr)
		{
			return;
		}
		http::Request *request = std::move(this->mRequest).release();
		http::Response *response = std::move(this->mResponse).release();
		if (response->Code() != HttpStatus::OK && response->Code() != HttpStatus::FOUND)
		{
			//LOG_ERROR("http request : {}", request->ToString())
			//LOG_ERROR("http response : {}", response->ToString())
		}

#ifdef ONLY_MAIN_THREAD
		this->mComponent->OnMessage(request, response);
#else
		Asio::Context &io = joke::App::Inst()->GetContext();
		io.post([this, request, response]
				{ this->mComponent->OnMessage(request, response); });
#endif
	}

	void RequestClient::OnReadError(const Asio::Code &code)
	{
		if (this->mRecvBuffer.size() == 0)
		{
			if (code == asio::error::eof)
			{
				this->OnComplete(HttpStatus::OK);
				return;
			}
			this->OnComplete(HttpStatus::BAD_REQUEST);
		}
	}

	void RequestClient::OnReceiveLine(std::istream &is, size_t size)
	{
		this->OnReceiveMessage(is, size);
	}

	void RequestClient::OnReceiveMessage(std::istream &is, size_t size)
	{
		int flag = this->mResponse->OnRecvMessage(is, size);
		switch (flag)
		{
		case tcp::ReadDone: // 完成
			this->OnComplete(HttpStatus::OK);
			break;
		case tcp::ReadOneLine: // 读一行
			this->ReadLine();
			break;
		case tcp::ReadSomeMessage: // 读一些
			this->ReadSome();
			break;
		case tcp::ReadError: // 消息错误
			this->OnComplete(HttpStatus::INTERNAL_SERVER_ERROR);
			break;
		case tcp::PacketLong:
		case tcp::ReadDecodeError: // 解析错误
			this->OnComplete(HttpStatus::BAD_REQUEST);
			break;
		default:
			this->ReadLength(flag);
			break;
		}
	}
}
