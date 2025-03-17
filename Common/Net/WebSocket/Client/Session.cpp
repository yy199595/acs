//
// Created by 64658 on 2025/1/2.
//
#include "XCode/XCode.h"
#include "Session.h"
#include "Http/Common/HttpRequest.h"
#include "Http/Common/HttpResponse.h"
#include "Util/Tools/String.h"
#include "Util/Crypt/sha1.h"
#include "Proto/Bson/base64.h"
namespace ws
{
	std::string generateSecWebSocketAccept(const std::string& secWebSocketKey) {
		const std::string GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
		std::string keyConcat = secWebSocketKey + GUID;

		std::string hash = help::Sha1::GetHash(keyConcat);
		return _bson::base64::encode(hash);
	}
}

namespace ws
{

	Session::Session(int id, Component * component, Asio::Context & main)
		: tcp::Client(1024 * 10), mSockId(id), mIsHttp(true), mComponent(component), mMainContext(main)
	{
		this->mPlayerId = 0;
		this->mHttpRequest = nullptr;
	}

	Session::~Session() noexcept
	{
		delete this->mHttpRequest;
	}

	void Session::Stop()
	{
		Asio::Context & context = this->mSocket->GetContext();
		std::shared_ptr<Client> self = this->shared_from_this();
		asio::post(context, [self, this]() { this->Close(XCode::CloseSocket); });
	}

	void Session::StartReceive(tcp::Socket* tcpSocket)
	{
		this->SetSocket(tcpSocket);
		Asio::Context & context = tcpSocket->GetContext();
		asio::post(context, [this, self = this->shared_from_this()]() {
			this->ReadLine();
			this->StartTimer(5, tcp::timeout::read);
		});
	}

	void Session::Send(rpc::Message* message)
	{
		Asio::Context & context = this->mSocket->GetContext();
		asio::post(context, [this, self = this->shared_from_this(), message] ()
		{
			this->mStream.str("");
			std::unique_ptr<ws::Message> wsMessage = std::make_unique<ws::Message>();
			{
				message->OnSendMessage(this->mStream);
				wsMessage->SetBody(ws::OPCODE_BIN, this->mStream.str(), false);
			}
			this->AddToSendQueue(std::move(wsMessage));
			delete message;
		});
	}

	void Session::Send(ws::Message* message)
	{
		Asio::Context & context = this->mSocket->GetContext();
		asio::post(context, [this, self = this->shared_from_this(), message] ()
		{
			this->AddToSendQueue(std::unique_ptr<ws::Message>(message));
		});
	}


	void Session::OnReceiveLine(std::istream& readStream, size_t size)
	{
		asio::error_code code;
		if(this->mHttpRequest == nullptr)
		{
			this->mHttpRequest = new http::Request();
		}
		this->OnReceiveMessage(readStream, size, code);
	}

	void Session::OnReceiveMessage(std::istream& readStream, size_t size, const asio::error_code& code)
	{
		if(code.value() != Asio::OK || size <= 0)
		{
			return;
		}
		int flag = 0;
		if(this->mIsHttp)
		{
			flag = this->mHttpRequest->OnRecvMessage(readStream, size);
			if(flag == tcp::read::done || flag == tcp::read::pause)
			{
				if (!this->DecodeByHttp())
				{
					this->Close(XCode::UnKnowPacket);
					return;
				}
				return;
			}
		}
		else
		{
			this->StopTimer();
			flag = this->mMessage->OnRecvMessage(readStream, size);
			if(flag == tcp::read::done)
			{
				this->OnReadBody();
				return;
			}
		}
		switch(flag)
		{
			case tcp::read::line:
				this->ReadLine();
				break;
			case tcp::read::some:
				this->ReadSome();
				break;
			case tcp::read::error:
				this->Close(XCode::UnKnowPacket);
				break;
			case tcp::read::big_long:
				this->Close(XCode::NetBigDataShutdown);
				break;
			default:
				this->ReadLength(flag);
				break;
		}
	}

	void Session::OnReadBody()
	{
		switch(this->mMessage->GetHeader().mOpCode)
		{
			case ws::OPCODE_BIN:
			case ws::OPCODE_TEXT:
			{
				if(!this->OnMessage())
				{
					this->Close(XCode::UnKnowPacket);
					return;
				}
				this->StartReceiveWebSocket();
				break;
			}
			case ws::OPCODE_CLOSE:
				this->Close(XCode::Ok);
				break;
			case ws::OPCODE_PING:
				this->OnPing();
				this->StartReceiveWebSocket();
				break;
			default:
				this->Close(XCode::UnKnowPacket);
				break;
		}
	}

	void Session::OnPing()
	{
		std::unique_ptr<ws::Message> pongMessage = std::make_unique<ws::Message>();
		{
			std::string message("pong");
			pongMessage->SetBody(ws::OPCODE_PONG, message);
			this->AddToSendQueue(std::move(pongMessage));
		}
	}

	bool Session::OnMessage()
	{
		std::shared_ptr<Client> self = this->shared_from_this();
		const std::string & message = this->mMessage->GetMessageBody();
		std::unique_ptr<rpc::Message> request = std::make_unique<rpc::Message>();
		switch(this->mMessage->GetHeader().mOpCode)
		{
			case ws::OPCODE_BIN:
			{
				if(!request->Decode(message.c_str(), (int)message.size()))
				{
					return false;
				}
				break;
			}
			case ws::OPCODE_TEXT:
			{
				if(!request->DecodeFromJson(message.c_str(), message.size()))
				{
					return false;
				}
				break;
			}
		}

		request->SetNet(rpc::Net::Ws);
		request->SetSockId(this->mSockId);
		if(this->mPlayerId > 0)
		{
			request->GetHead().Add(rpc::Header::player_id, this->mPlayerId);
		}
		request->GetHead().Add(rpc::Header::client_sock_id, this->mSockId);
		asio::post(this->mMainContext, [self, this, req = request.release()]
		{
			this->mComponent->OnMessage(this->mSockId, req, nullptr);
		});
		this->mMessage->Clear();
		return true;
	}

	void Session::Close(int code)
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
			std::shared_ptr<Client> self = this->shared_from_this();
			asio::post(this->mMainContext, [self, this, code, id = this->mSockId]()
			{
				this->mComponent->OnClientError(id, code);
			});
		}
		this->mSockId = 0;
	}

	bool Session::DecodeByHttp()
	{
		if (!this->mHttpRequest->IsMethod("GET"))
		{
			return false;
		}
		const http::Head & head = this->mHttpRequest->ConstHeader();
		if(!head.IsEqual("Upgrade", "websocket"))
		{
			return false;
		}
		int version = 0;
		std::string value;
		if(!head.Get("Sec-WebSocket-Key", value))
		{
			return false;
		}
		if(!head.Get("Sec-WebSocket-Version", version))
		{
			return false;
		}
		http::Response httpResponse;
		std::string shakeValue = ws::generateSecWebSocketAccept(value);
		{
			httpResponse.Header().Add("Connection", "Upgrade");
			httpResponse.Header().Add("Upgrade", "websocket");
			httpResponse.SetCode(HttpStatus::SWITCHING_PROTOCOLS);
			httpResponse.Header().Add("Sec-WebSocket-Accept", shakeValue);
			if(!this->SendSync(httpResponse))
			{
				return false;
			}
		}
		this->mIsHttp = false;
		this->StartReceiveWebSocket();
		return true;
	}

	void Session::StartReceiveWebSocket()
	{
		this->mMessage = std::make_unique<ws::Message>();
		this->ReadSome();
	}

	void Session::OnSendMessage(size_t size)
	{
		if(!this->mWaitSendMessage.empty())
		{
			this->mWaitSendMessage.pop();
			if(!this->mWaitSendMessage.empty())
			{
				this->Write(*this->mWaitSendMessage.front());
			}
		}
	}

	void Session::AddToSendQueue(std::unique_ptr<ws::Message> message)
	{
		this->mWaitSendMessage.emplace(std::move(message));
		if(this->mWaitSendMessage.size() == 1)
		{
			this->Write(*this->mWaitSendMessage.front());
		}
	}

	void Session::OnReadError(const Asio::Code& code)
	{
		CONSOLE_LOG_ERROR("[{}] => {}", this->mSocket->GetAddress(), code.message())
		this->Close(XCode::NetReadFailure);
	}

	void Session::OnSendMessage(const Asio::Code& code)
	{
		this->Close(XCode::NetSendFailure);
	}
}