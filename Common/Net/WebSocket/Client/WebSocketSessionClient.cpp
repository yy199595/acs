//
// Created by 64658 on 2025/1/2.
//
#include "XCode/XCode.h"
#include "WebSocketSessionClient.h"
#include "Http/Common/HttpRequest.h"
#include "Http/Common/HttpResponse.h"
#include "Util/Tools/String.h"

#include <openssl/sha.h> // 用于计算 SHA-1
#include <openssl/bio.h>
#include <openssl/evp.h>


namespace ws
{
	std::string base64Encode(const unsigned char* input, size_t length) {
		BIO* b64 = BIO_new(BIO_f_base64());
		BIO* bio = BIO_new(BIO_s_mem());
		b64 = BIO_push(b64, bio);

		BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL); // 不换行
		BIO_write(b64, input, length);
		BIO_flush(b64);

		BUF_MEM* buffer;
		BIO_get_mem_ptr(b64, &buffer);
		std::string result(buffer->data, buffer->length);

		BIO_free_all(b64);
		return result;
	}

	std::string generateSecWebSocketAccept(const std::string& secWebSocketKey) {
		const std::string GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
		std::string keyConcat = secWebSocketKey + GUID;

		// 计算 SHA-1 哈希
		unsigned char hash[SHA_DIGEST_LENGTH];
		SHA1(reinterpret_cast<const unsigned char*>(keyConcat.c_str()), keyConcat.length(), hash);

		// Base64 编码
		return base64Encode(hash, SHA_DIGEST_LENGTH);
	}
}

namespace ws
{

	SessionClient::SessionClient(int id, Component * component, Asio::Context & main)
		: tcp::Client(1024 * 10), mSockId(id), mIsHttp(true), mComponent(component), mMainContext(main)
	{
		this->mPlayerId = 0;
		this->mHttpRequest = nullptr;
	}

	SessionClient::~SessionClient() noexcept
	{
		delete this->mHttpRequest;
		printf("===========\n");
	}

	void SessionClient::Stop()
	{
		Asio::Context & context = this->mSocket->GetContext();
		std::shared_ptr<Client> self = this->shared_from_this();
		asio::post(context, [self, this]() { this->Close(XCode::CloseSocket); });
	}

	void SessionClient::StartReceive(tcp::Socket* tcpSocket)
	{
		this->mSocket.reset(tcpSocket);
		Asio::Context & context = tcpSocket->GetContext();
		std::shared_ptr<Client> self = this->shared_from_this();
		asio::post(context, [self, this]() { this->ReadLine(); });
	}

	void SessionClient::StartWrite(rpc::Message* message)
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

	void SessionClient::OnReceiveLine(std::istream& readStream, size_t size)
	{
		asio::error_code code;
		if(this->mHttpRequest == nullptr)
		{
			this->mHttpRequest = new http::Request();
		}
		this->OnReceiveMessage(readStream, size, code);
	}

	void SessionClient::OnReceiveMessage(std::istream& readStream, size_t size, const asio::error_code& code)
	{
		if(code.value() != Asio::OK || size <= 0)
		{
			return;
		}
		int flag = 0;
		if(this->mIsHttp)
		{
			flag = this->mHttpRequest->OnRecvMessage(readStream, size);
			if(flag == tcp::ReadDone || flag == tcp::ReadPause)
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
			flag = this->mMessage->OnRecvMessage(readStream, size);
			if(flag == tcp::ReadDone)
			{
				this->OnReadBody();
				return;
			}
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
			default:
				this->ReadLength(flag);
				break;
		}
	}

	void SessionClient::OnReadBody()
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
				break;
			default:
				this->Close(XCode::UnKnowPacket);
				break;
		}
	}

	void SessionClient::OnPing()
	{
		std::unique_ptr<ws::Message> pongMessage = std::make_unique<ws::Message>();
		{
			std::string message("pong");
			pongMessage->SetBody(ws::OPCODE_PONG, message);
			this->AddToSendQueue(std::move(pongMessage));
		}
	}

	bool SessionClient::OnMessage()
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

	void SessionClient::Close(int code)
	{
		if(this->mSockId == 0)
		{
			return;
		}

		this->StopTimer();
		this->mSocket->Close();
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

	bool SessionClient::DecodeByHttp()
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
		this->StartUpdate(30);
		this->StartReceiveWebSocket();
		CONSOLE_LOG_ERROR("[{}] connect ok", this->mSocket->GetAddress())
		return true;
	}

	void SessionClient::StartReceiveWebSocket()
	{
		this->mMessage = std::make_unique<ws::Message>();
		this->ReadSome();
	}

	void SessionClient::OnUpdate()
	{

	}

	void SessionClient::OnSendMessage()
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

	void SessionClient::AddToSendQueue(std::unique_ptr<ws::Message> message)
	{
		this->mWaitSendMessage.emplace(std::move(message));
		if(this->mWaitSendMessage.size() == 1)
		{
			this->Write(*this->mWaitSendMessage.front());
		}
	}

	void SessionClient::OnReadError(const Asio::Code& code)
	{
		this->Close(XCode::NetReadFailure);
	}

	void SessionClient::OnSendMessage(const Asio::Code& code)
	{
		this->Close(XCode::NetSendFailure);
	}
}