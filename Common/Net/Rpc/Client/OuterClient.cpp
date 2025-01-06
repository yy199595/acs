//
// Created by mac on 2021/11/28.
//

#include"OuterClient.h"
#include"XCode/XCode.h"
#include"Util/Tools/TimeHelper.h"

namespace rpc
{
	OuterClient::OuterClient(int id, Component * component, Asio::Context & main)
		: Client(rpc::OuterBufferMaxSize), mSockId(id), mMainContext(main), mClose(false), mPlayerId(0)
	{
		this->mMessage = nullptr;
		this->mLastRecvTime = 0;
		this->mComponent = component;
		this->mDecodeState = tcp::Decode::None;
	}

	OuterClient::~OuterClient() noexcept
	{
		while(!this->mSendMessages.empty())
		{
			delete this->mSendMessages.front();
			this->mSendMessages.pop();
		}
	}

	void OuterClient::Stop()
	{
		Asio::Context & context = this->mSocket->GetContext();
		std::shared_ptr<Client> self = this->shared_from_this();
		asio::post(context, [this, self] { this->CloseSocket(); });
	}

	void OuterClient::StartReceive(tcp::Socket * socket, int second)
	{
		this->SetSocket(socket);
		std::shared_ptr<Client> self = this->shared_from_this();
		asio::post(this->mSocket->GetContext(), [this, self, second]
		{
			this->ReadLength(rpc::RPC_PACK_HEAD_LEN, second);
		});
	}

	void OuterClient::OnTimeout(tcp::TimeoutFlag flag)
	{
		long long nowTime = help::Time::NowSec();
		if(nowTime - this->mLastRecvTime >= 30)
		{
			this->CloseSocket(XCode::NetTimeout);
		}
	}

	void OuterClient::OnReadError(const Asio::Code& code)
	{
		this->CloseSocket(XCode::NetReadFailure);
	}

    void OuterClient::OnReceiveMessage(std::istream & readStream, size_t size, const Asio::Code & code)
	{
		if (size <= 0 || code.value() != Asio::OK)
		{
			return;
		}
		switch (this->mDecodeState)
		{
			case tcp::Decode::None:
			{
				tcp::Data::Read(readStream, this->mProtoHead);
				if (this->mProtoHead.Len >= this->mMaxCount)
				{
					this->CloseSocket(XCode::NetBigDataShutdown);
					return;
				}
				this->mMessage = std::make_unique<rpc::Message>();
				{
					this->mMessage->Init(this->mProtoHead);
				}
				this->mDecodeState = tcp::Decode::MessageBody;
				this->ReadLength(this->mProtoHead.Len);
				break;
			}
			case tcp::Decode::MessageBody:
			{
				if (this->mMessage->OnRecvMessage(readStream, size) != 0)
				{
					this->CloseSocket();
					return;
				}
				this->mDecodeState = tcp::Decode::Done;
				break;
			}
		}
		if (this->mDecodeState != tcp::Decode::Done)
		{
			return;
		}
		rpc::Message* request = this->mMessage.release();
		{
			request->SetSockId(this->mSockId);
			request->GetHead().Add(rpc::Header::player_id, this->mPlayerId);
			request->GetHead().Add(rpc::Header::client_sock_id, this->mSockId);
		}
		switch(request->GetType())
		{
			case rpc::Type::Ping:
			{
				rpc::Message * pongMessage = new rpc::Message();
				{
					pongMessage->SetType(rpc::Type::Pong);
					this->AddToSendQueue(pongMessage);
				}
				break;
			}
			case rpc::Type::Request:
			{
#ifdef __DEBUG__
				request->TempHead().Add(rpc::Header::from_addr, this->mSocket->GetAddress());
#endif

				std::shared_ptr<Client> self = this->shared_from_this();
				asio::post(this->mMainContext, [this, self, request]
				{
					this->mComponent->OnMessage(request, nullptr);
				});
				break;
			}
			default:
			{
				this->CloseSocket(XCode::UnKnowPacket);
				return;
			}
		}

		this->mDecodeState = tcp::Decode::None;
		this->mLastRecvTime = help::Time::NowSec();
		Asio::Context & context = this->mSocket->GetContext();
		std::shared_ptr<Client> self = this->shared_from_this();
		asio::post(context, [this, self] { this->ReadLength(rpc::RPC_PACK_HEAD_LEN); });
	}

    void OuterClient::CloseSocket()
	{
		if(this->mClose) {
			return;
		}
		this->StopTimer();
		while(!this->mSendMessages.empty())
		{
			delete this->mSendMessages.front();
			this->mSendMessages.pop();
		}
		this->mClose = true;
		this->mSocket->Close();
	}

	void OuterClient::SendFirstMessage()
	{
		if (!this->mSendMessages.empty())
		{
			this->Write(*this->mSendMessages.front());
		}
	}

	void OuterClient::CloseSocket(int code)
	{
		if(!this->mClose)
		{
			this->CloseSocket();
			std::shared_ptr<Client> self = this->shared_from_this();
			asio::post(this->mMainContext, [this, self, code]() {
				this->mComponent->OnClientError(this->mSockId, code);
			});
		}
	}

	void OuterClient::OnSendMessage(size_t size)
	{
		if (!this->mSendMessages.empty())
		{
			rpc::Message* message = this->mSendMessages.front();
			if (message->GetType() == rpc::Type::Response)
			{
				if(message->GetCode() == XCode::CloseSocket)
				{
					this->CloseSocket(XCode::CloseSocket);
					return;
				}
			}
			delete message;
			this->mSendMessages.pop();
			this->SendFirstMessage();
		}
	}

	void OuterClient::OnSendMessage(const asio::error_code& code)
	{
		this->CloseSocket(XCode::SendMessageFail);
	}

	void OuterClient::Send(rpc::Message* message)
	{
		std::shared_ptr<Client> self = this->shared_from_this();
		asio::post(this->mSocket->GetContext(), [this, self, message]
		{
			this->AddToSendQueue(message);
		});
	}

	void OuterClient::AddToSendQueue(rpc::Message* message)
	{
		this->mSendMessages.emplace(message);
		if(this->mSendMessages.size() == 1)
		{
			this->Write(*message);
		}
	}
}