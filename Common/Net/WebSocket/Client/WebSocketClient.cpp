//
// Created by 64658 on 2025/1/2.
//

#include "WebSocketClient.h"
#include "XCode/XCode.h"
#include "Http/Common/HttpRequest.h"
#include "Http/Common/HttpResponse.h"

#include <openssl/sha.h>
#include <openssl/bio.h>

namespace ws
{

	std::string CalculateSecWebSocketAccept(const std::string& key) {
		std::string input = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

		unsigned char hash[SHA_DIGEST_LENGTH];
		SHA1(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);

		char output[2 * SHA_DIGEST_LENGTH + 1];
		for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
			sprintf(output + 2 * i, "%02x", static_cast<int>(hash[i]));
		}
		output[2 * SHA_DIGEST_LENGTH] = '\0';

		return std::string(output);
	}

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
			if(!this->mWaitSendMessage.empty())
			{
				ws::Message * message = this->mWaitSendMessage.front();
				{
					this->Write(*message);
				}
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
		std::string secWebSocketAccept = ws::CalculateSecWebSocketAccept("dGhlIHNhbXBsZSBub25jZQ==");
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
				delete this->mHttpResponse;
				this->mHttpResponse = nullptr;
				this->ReadSome();
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