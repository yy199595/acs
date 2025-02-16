//
// Created by 64658 on 2025/1/2.
//

#include "Client.h"
#include "XCode/XCode.h"
#include "Http/Common/HttpRequest.h"
#include "Http/Common/HttpResponse.h"
#include "Util/Tools/String.h"
#include "Proto/Bson/base64.h"
#include "Util/Crypt/sha1.h"
namespace ws
{

	std::string ComputeWebSocketAccept(const std::string& secWebSocketKey) {
		const std::string websocketGUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
		std::string keyWithGUID = secWebSocketKey + websocketGUID;

		// 计算 SHA-1 哈希
		std::string hash = help::Sha1::GetHash(keyWithGUID);
		return _bson::base64::encode(hash);
	}
}

namespace ws
{
	Client::Client(int id, ws::Component* component, Asio::Context& main)
			: tcp::Client(1024 * 10), mSockId(id), mComponent(component), mMainContext(main)
	{
		this->mHttpRequest = nullptr;
		this->mHttpResponse = nullptr;
	}

	void Client::OnSendMessage(size_t size)
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
			this->mWaitSendMessage.pop();
			this->SendFirstMessage();
		}
	}

	void Client::OnUpdate()
	{
		std::unique_ptr<ws::Message> pingMessage = std::make_unique<ws::Message>();
		{
			std::string message("ping");
			pingMessage->SetBody(ws::OPCODE_PING, message);
			this->mWaitSendMessage.emplace(std::move(pingMessage));
			if(this->mWaitSendMessage.size() == 1)
			{
				this->Write(*this->mWaitSendMessage.front());
			}
		}
	}

	void Client::SendFirstMessage()
	{
		if(!this->mWaitSendMessage.empty())
		{
			this->Write(*this->mWaitSendMessage.front());
		}
	}

	void Client::OnConnect(const Asio::Code &code, int count)
	{
		if(code.value() != Asio::OK)
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
		this->ClearSendStream();
		this->Write(*this->mHttpRequest);
	}

	void Client::Close()
	{
		Asio::Context & context = this->mSocket->GetContext();
		asio::post(context, [this, self = this->shared_from_this()] ()
		{
			this->Close(XCode::CloseSocket);
		});
	}

	void Client::Close(int code)
	{
		if(this->mSockId == 0)
		{
			return;
		}

		this->StopTimer();
		this->mSocket->Close();
		this->ClearSendStream();
		this->ClearRecvStream();
		while(!this->mWaitSendMessage.empty())
		{
			this->mWaitSendMessage.pop();
		}
		if(code != XCode::CloseSocket)
		{
			auto self = this->shared_from_this();
			asio::post(this->mMainContext, [self, this, code, id = this->mSockId]()
			{
				this->mComponent->OnClientError(id, code);
			});
		}
		this->mSockId = 0;
	}

	void Client::Send(ws::Message* message)
	{
		Asio::Context & context = this->mSocket->GetContext();
		asio::post(context, [this, self = this->shared_from_this(), message] ()
		{
			std::unique_ptr<ws::Message> wsMessage(message);
			this->AddToSendQueue(std::move(wsMessage));
		});

	}

	void Client::AddToSendQueue(std::unique_ptr<ws::Message> wsMessage)
	{
		this->mWaitSendMessage.emplace(std::move(wsMessage));
		if(this->mWaitSendMessage.size() == 1)
		{
			this->Write(*this->mWaitSendMessage.front());
		}
	}

	void Client::Send(rpc::Message* message)
	{
		Asio::Context & context = this->mSocket->GetContext();
		asio::post(context, [this, self = this->shared_from_this(), message] ()
		{
			this->mStream.str("");
			std::unique_ptr<ws::Message> wsMessage = std::make_unique<ws::Message>();
			{
				message->OnSendMessage(this->mStream);
				wsMessage->SetBody(ws::OPCODE_BIN, this->mStream.str(), true);
			}
			delete message;
			this->AddToSendQueue(std::move(wsMessage));
		});
	}

	void Client::OnReadError(const Asio::Code& code)
	{
		this->Close(XCode::NetReadFailure);
	}

	void Client::OnSendMessage(const Asio::Code& code)
	{
		this->Connect(5);
	}

	void Client::OnReceiveLine(std::istream& readStream, size_t size)
	{
		asio::error_code code;
		this->OnReceiveMessage(readStream, size, code);
	}

	void Client::OnReceiveMessage(std::istream& readStream, size_t size, const asio::error_code& code)
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
					this->StartUpdate(ws::PING_TIME);
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
			case tcp::PacketLong:
				this->Close(XCode::NetBigDataShutdown);
				break;
			case tcp::ReadDone:
			{
				auto self = this->shared_from_this();
				const std::string & message = this->mMessage->GetMessageBody();
				std::unique_ptr<rpc::Message> request = std::make_unique<rpc::Message>();
				if(!request->Decode(message.c_str(), (int)message.size()))
				{
					this->Close(XCode::UnKnowPacket);
					return;
				}
				request->SetNet(rpc::Net::Ws);
				request->SetSockId(this->mSockId);
				asio::post(this->mMainContext, [self, this, req = request.release()]
				{
					this->mComponent->OnMessage(req, nullptr);
				});
				this->mMessage->Clear();
				this->ReadSome();
				break;
			}
			default:
				this->ReadLength(flag);
				break;
		}
	}
}