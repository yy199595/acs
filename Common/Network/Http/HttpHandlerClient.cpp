//
// Created by zmhy0073 on 2022/1/19.
//

#include"HttpHandlerClient.h"

namespace Sentry
{
	HttpHandlerClient::HttpHandlerClient(std::shared_ptr<SocketProxy> socketProxy)
		: Tcp::TcpContext(socketProxy)
	{
		this->mHttpResponse = std::make_shared<HttpHandlerResponse>();
		this->mHttpRequest = std::make_shared<HttpHandlerRequest>(socketProxy->GetAddress());
	}

	std::shared_ptr<HttpHandlerRequest> HttpHandlerClient::Read()
	{
		this->mReadTask = std::make_shared<TaskSource<bool>>();
#ifdef ONLY_MAIN_THREAD
		this->ReadData();
#else
		IAsioThread & netWorkThread = this->mSocket->GetThread();
		netWorkThread.Invoke(&HttpHandlerClient::ReadData, this);
#endif
		return this->mReadTask->Await() ? this->mHttpRequest : nullptr;
	}

	bool HttpHandlerClient::Writer(HttpStatus code, Json::Writer& jsonWriter)
	{
		this->mHttpResponse->Write(code, jsonWriter);
		this->mWriteTask = std::make_shared<TaskSource<bool>>();
#ifdef ONLY_MAIN_THREAD
		this->Send(this->mHttpResponse);
#else
		IAsioThread& netWorkThread = this->mSocket->GetThread();
		netWorkThread.Invoke(&HttpHandlerClient::Send, this, this->mHttpResponse);
#endif
		return this->mWriteTask->Await();
	}

	void HttpHandlerClient::ReadData()
	{
		AsioTcpSocket& tcpSocket = this->mSocket->GetSocket();
		asio::async_read(tcpSocket, this->mStreamBuffer, asio::transfer_at_least(1),
			[this](const asio::error_code& code, size_t size)
			{
				if (code)
				{
#ifdef __NET_ERROR_LOG__
					CONSOLE_LOG_ERROR(code.message());
#endif
					this->mReadTask->SetResult(false);
					return;
				}
				HttpStatus httpCode = this->mHttpRequest->OnReceiveData(this->mStreamBuffer);
				switch (httpCode)
				{
				case HttpStatus::CONTINUE:
					this->ReadData();
					break;
				case HttpStatus::OK:
					this->mReadTask->SetResult(true);
					break;
				default:
					this->mReadTask->SetResult(false);
					break;
				}
			});
	}

	void HttpHandlerClient::OnSendMessage(const asio::error_code& code, std::shared_ptr<Tcp::ProtoMessage> message)
	{
		if(code)
		{
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR(code.message());
#endif
			this->mWriteTask->SetResult(false);
			return;
		}
		this->mWriteTask->SetResult(true);
	}
}