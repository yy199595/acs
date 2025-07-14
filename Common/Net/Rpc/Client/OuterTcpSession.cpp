//
// Created by mac on 2021/11/28.
//

#include"OuterTcpSession.h"
#include"XCode/XCode.h"
#include"Util/Tools/TimeHelper.h"

namespace rpc
{
	OuterTcpSession::OuterTcpSession(int id, Component * component, Asio::Context & main)
		: Client(rpc::OUTER_RPC_BODY_MAX_LENGTH), mSockId(id), mMainContext(main), mPlayerId(0)
	{
		this->mRecvCount = 0;
		this->mMessage = nullptr;
		this->mLastRecvTime = 0;
		this->mComponent = component;
		this->mDecodeState = tcp::Decode::None;
	}

	OuterTcpSession::~OuterTcpSession() noexcept
	{
		while(!this->mSendMessages.empty())
		{
			this->mSendMessages.pop();
		}
	}

	void OuterTcpSession::Stop()
	{
		Asio::Context & context = this->mSocket->GetContext();
		std::shared_ptr<Client> self = this->shared_from_this();
		asio::post(context, [this, self] { this->CloseSocket(); });
	}

	void OuterTcpSession::StartReceive(tcp::Socket * socket, int second)
	{
		this->SetSocket(socket);
		std::shared_ptr<Client> self = this->shared_from_this();
		asio::post(this->mSocket->GetContext(), [this, self, second]
		{
			this->ReadLength(rpc::RPC_PACK_HEAD_LEN, second);
		});
	}

	void OuterTcpSession::OnReadError(const Asio::Code& code)
	{
		this->CloseSocket(XCode::NetReadFailure);
	}

    void OuterTcpSession::OnReceiveMessage(std::istream & readStream, size_t size, const Asio::Code & code)
	{
		if (size <= 0 || code.value() != Asio::OK)
		{
			return;
		}
		switch (this->mDecodeState)
		{
			case tcp::Decode::None:
			{
				tcp::Data::ReadHead(readStream, this->mProtoHead, true);
				if (this->mProtoHead.Len >= rpc::OUTER_RPC_BODY_MAX_LENGTH)
				{
					this->CloseSocket(XCode::NetBigDataShutdown);
					return;
				}
				this->mMessage = std::make_unique<rpc::Message>();
				{
					this->mMessage->Init(this->mProtoHead);
				}
				size_t readCount = 0;
				this->mDecodeState = tcp::Decode::MessageBody;
				if(this->mSocket->CanRecvCount(readCount))
				{
					this->CloseSocket(XCode::NetReadFailure);
					return;
				}
				if(readCount < this->mProtoHead.Len)
				{
					this->CloseSocket(XCode::NetReadFailure);
					return;
				}
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
			request->GetHead().Add(rpc::Header::id, this->mPlayerId);
			request->GetHead().Add(rpc::Header::client_sock_id, this->mSockId);
		}
		switch(request->GetType())
		{
			case rpc::type::ping:
			{
				std::unique_ptr<rpc::Message> pongMessage = std::make_unique<rpc::Message>();
				{
					pongMessage->SetType(rpc::type::pong);
					this->AddToSendQueue(pongMessage);
				}
				break;
			}
			case rpc::type::request:
			{
				if (request->GetBody().size() >= rpc::OUTER_RPC_BODY_MAX_LENGTH)
				{
					this->CloseSocket(XCode::NetBigDataShutdown);
					return;
				}
#ifdef __DEBUG__
				std::string address = this->GetAddress();
				request->TempHead().Add(rpc::Header::from_addr, address);
#endif
				this->mRecvCount++;
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

    void OuterTcpSession::CloseSocket()
	{
		if(this->mSocket->IsActive())
		{
			this->StopTimer();
			while(!this->mSendMessages.empty())
			{
				this->mSendMessages.pop();
			}
			this->mSocket->Close();
		}
	}

	void OuterTcpSession::SendFirstMessage()
	{
		if (!this->mSendMessages.empty())
		{
			this->Write(*this->mSendMessages.front());
		}
	}

	void OuterTcpSession::CloseSocket(int code)
	{
		if(this->mSocket->IsActive())
		{
			this->CloseSocket();
			std::shared_ptr<Client> self = this->shared_from_this();
			asio::post(this->mMainContext, [this, self, code]() {
				this->mComponent->OnClientError(this->mSockId, code);
			});
		}
	}

	void OuterTcpSession::OnSendMessage(size_t size)
	{
		if (!this->mSendMessages.empty())
		{
			std::unique_ptr<rpc::Message> & message = this->mSendMessages.front();
			{
				if (message->GetType() == rpc::type::response)
				{
					if(message->GetCode() == XCode::CloseSocket)
					{
						this->CloseSocket(XCode::CloseSocket);
						return;
					}
				}
				this->mSendMessages.pop();
				this->SendFirstMessage();
			}
		}
	}

	void OuterTcpSession::OnSendMessage(const asio::error_code& code)
	{
		this->CloseSocket(XCode::SendMessageFail);
	}

	void OuterTcpSession::Send(std::unique_ptr<rpc::Message>& message)
	{
		std::shared_ptr<Client> self = this->shared_from_this();
		asio::post(this->mSocket->GetContext(), [this, self, req = message.release()]
		{
			std::unique_ptr<rpc::Message> message(req);
			this->AddToSendQueue(message);
		});
	}

	void OuterTcpSession::AddToSendQueue(std::unique_ptr<rpc::Message> & message)
	{
		this->mSendMessages.emplace(std::move(message));
		if(this->mSendMessages.size() == 1)
		{
			this->Write(*this->mSendMessages.front());
		}
	}
}