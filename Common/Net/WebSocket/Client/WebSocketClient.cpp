//
// Created by 64658 on 2025/1/2.
//

#include "WebSocketClient.h"
#include "XCode/XCode.h"
#include "Http/Common/HttpRequest.h"
#include "Http/Common/HttpResponse.h"

#include <openssl/sha.h>
#include <openssl/bio.h>
#include "Util/Tools/String.h"
namespace ws
{
	// Base64 编码函数
	std::string Base64Encode(const unsigned char* input, int length) {
		BIO* bmem = BIO_new(BIO_s_mem());
		BIO* b64 = BIO_new(BIO_f_base64());
		b64 = BIO_push(b64, bmem);

		// 设置为无换行模式
		BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
		BIO_write(b64, input, length);
		BIO_flush(b64);

		BUF_MEM* bptr;
		BIO_get_mem_ptr(b64, &bptr);

		std::string result(bptr->data, bptr->length);
		BIO_free_all(b64);
		return result;
	}

	std::string ComputeWebSocketAccept(const std::string& secWebSocketKey) {
		const std::string websocketGUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
		std::string keyWithGUID = secWebSocketKey + websocketGUID;

		// 计算 SHA-1 哈希
		unsigned char sha1Result[SHA_DIGEST_LENGTH];
		SHA1(reinterpret_cast<const unsigned char*>(keyWithGUID.c_str()), keyWithGUID.size(), sha1Result);

		// 对哈希结果进行 Base64 编码
		return Base64Encode(sha1Result, SHA_DIGEST_LENGTH);
	}
}

namespace ws
{
	RequestClient::RequestClient(int id, ws::Component* component, Asio::Context& main)
			: tcp::Client(1024 * 10), mSockId(id), mComponent(component), mMainContext(main)
	{
		this->mHttpRequest = nullptr;
		this->mHttpResponse = nullptr;
	}

	void RequestClient::OnSendMessage()
	{
		if(this->mHttpRequest != nullptr)
		{
			this->mHttpResponse = new http::Response();
			{
				this->ReadLine();
				delete this->mHttpRequest;
				this->mHttpRequest = nullptr;
			}
		}
		else if(!this->mWaitSendMessage.empty())
		{
			delete this->mWaitSendMessage.front();
			this->mWaitSendMessage.pop();
			this->SendFirstMessage();
		}
	}

	void RequestClient::SendFirstMessage()
	{
		if(!this->mWaitSendMessage.empty())
		{
			ws::Message * message = this->mWaitSendMessage.front();
			{
				this->Write(*message);
			}
		}
	}

	void RequestClient::OnConnect(bool result, int count)
	{
		if(!result)
		{
			this->Close(XCode::NetConnectFailure);
			return;
		}
		this->mHttpRequest = new http::Request("GET");
		http::Head & head = this->mHttpRequest->Header();
		std::string randomKey = help::Str::RandomString(16);
		std::string secWebSocketAccept = ws::ComputeWebSocketAccept(randomKey);
		{
			head.Add("Connection", "Upgrade");
			head.Add("Upgrade", "websocket");
			head.Add("Sec-WebSocket-Key", secWebSocketAccept);
			head.Add("Sec-WebSocket-Version", 13);
		}
		this->Write(*this->mHttpRequest);
	}

	void RequestClient::Close(int code)
	{
		if(this->mSockId == 0)
		{
			return;
		}

		this->StopTimer();
		this->mSocket->Close();
		std::shared_ptr<Client> self = this->shared_from_this();
		asio::post(this->mMainContext, [self, this, code, id = this->mSockId]()
		{
			while(!this->mWaitSendMessage.empty())
			{
				ws::Message * message = this->mWaitSendMessage.front();
				this->mComponent->OnSendFailure(id, message);
				this->mWaitSendMessage.pop();
			}
			this->mComponent->OnClientError(id, code);
		});
		this->mSockId = 0;
	}

	void RequestClient::StartWrite(ws::Message* message)
	{
		Asio::Context & context = this->mSocket->GetContext();
		asio::post(context, [this, self = this->shared_from_this(), message] ()
		{
			this->mWaitSendMessage.emplace(message);
			if(this->mWaitSendMessage.size() == 1)
			{
				this->Write(*message);
			}
		});
	}

	void RequestClient::OnReadError(const Asio::Code& code)
	{
		this->Close(XCode::NetReadFailure);
	}

	void RequestClient::OnSendMessage(const Asio::Code& code)
	{
		this->Connect(5);
	}

	void RequestClient::OnReceiveLine(std::istream& readStream, size_t size)
	{
		asio::error_code code;
		this->OnReceiveMessage(readStream, size, code);
	}

	void RequestClient::OnReceiveMessage(std::istream& readStream, size_t size, const asio::error_code& code)
	{
		int flag = 0;
		if(this->mHttpResponse != nullptr)
		{
			flag = this->mHttpResponse->OnRecvMessage(readStream, size);
			if(flag == tcp::ReadDone)
			{
				if(this->mHttpResponse->Code() != HttpStatus::SWITCHING_PROTOCOLS)
				{
					this->Close(XCode::Failure);
				}
				else
				{
					this->ReadSome();
					this->SendFirstMessage();
				}
				delete this->mHttpResponse;
				this->mHttpResponse = nullptr;
				return;
			}
		}
		else
		{
			if(this->mMessage == nullptr)
			{
				this->mMessage = std::make_unique<ws::Message>();
			}
			flag = this->mMessage->OnRecvMessage(readStream, size);
		}
		switch(flag)
		{
			case tcp::ReadOneLine:
				this->ReadLine();
				break;
			case tcp::ReadSome:
				this->ReadSome();
				break;
			case tcp::ReadError:
				this->Close(XCode::UnKnowPacket);
				break;
			case tcp::ReadDone:
			{
				ws::Message * request = this->mMessage.release();
				std::shared_ptr<tcp::Client> self = this->shared_from_this();
				asio::post(this->mMainContext, [request, this, self, id = this->mSockId]()
				{
					this->mComponent->OnMessage(id, request, nullptr);
				});
				this->ReadSome();
				break;
			}
			default:
				this->ReadLength(flag);
				break;
		}
	}
}