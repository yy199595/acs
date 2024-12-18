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
		if(this->mSocket == nullptr)
		{
			return;
		}
		Asio::Context & context = this->mSocket->GetContext();
		std::shared_ptr<Client> self = this->shared_from_this();
		asio::post(context, [this, self] { this->CloseSocket(); });
	}

	void OuterClient::StartReceive(tcp::Socket * socket, int second)
	{
		this->SetSocket(socket);
#ifdef ONLY_MAIN_THREAD
		this->ReadLength(rpc::RPC_PACK_HEAD_LEN, second);
#else
		std::shared_ptr<Client> self = this->shared_from_this();
		asio::post(this->mSocket->GetContext(), [this, self, second]
		{
			this->ReadLength(rpc::RPC_PACK_HEAD_LEN, second);
		});
#endif
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

	inline bool CheckProtoHead(const rpc::ProtoHead & data)
	{
		switch (data.Type)
		{
			case rpc::Type::Auth:
			case rpc::Type::Ping:
			case rpc::Type::Request:
				return true;
		}
		return false;
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

				if (!CheckProtoHead(this->mProtoHead))
				{
					this->CloseSocket(XCode::UnKnowPacket);
					return;
				}

				if (this->mProtoHead.Len >= this->mMaxCount)
				{
					this->CloseSocket(XCode::NetBigDataShutdown);
					return;
				}
				this->mMessage = std::make_unique<rpc::Packet>();
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
		rpc::Packet* request = this->mMessage.release();
		{
			request->SetSockId(this->mSockId);
			request->GetHead().Add(rpc::Header::player_id, this->mPlayerId);
			request->GetHead().Add(rpc::Header::rpc_id, request->GetRpcId());
			request->GetHead().Add(rpc::Header::client_sock_id, this->mSockId);
		}
#ifdef __DEBUG__
		request->TempHead().Add(rpc::Header::from_addr, this->mSocket->GetAddress());
#endif
#ifdef ONLY_MAIN_THREAD
		this->mComponent->OnMessage(request, nullptr);
#else
		std::shared_ptr<Client> self = this->shared_from_this();
		asio::post(this->mMainContext, [this, self, request] {
			this->mComponent->OnMessage(request, nullptr);
		});
#endif
		this->mDecodeState = tcp::Decode::None;
		this->mLastRecvTime = help::Time::NowSec();
		Asio::Context & context = this->mSocket->GetContext();
		std::shared_ptr<Client> that = this->shared_from_this();
		asio::post(context, [this, that] { this->ReadLength(rpc::RPC_PACK_HEAD_LEN); });
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
#ifdef ONLY_MAIN_THREAD
			this->mComponent->OnClientError(this->mSockId, code);
#else
			std::shared_ptr<Client> self = this->shared_from_this();
			asio::post(this->mMainContext, [this, self, code]() {
				this->mComponent->OnClientError(this->mSockId, code);
			});
#endif
		}
	}

	void OuterClient::OnSendMessage()
	{
		if (!this->mSendMessages.empty())
		{
			rpc::Packet* message = this->mSendMessages.front();
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

	bool OuterClient::Send(rpc::Packet* message)
	{
		LOG_CHECK_RET_FALSE(message);
#ifdef ONLY_MAIN_THREAD
		this->Write(*message);
#else
		std::shared_ptr<Client> self = this->shared_from_this();
		asio::post(this->mSocket->GetContext(), [this, self, message]
		{
			this->mSendMessages.emplace(message);
			if(this->mSendMessages.size() == 1)
			{
				this->Write(*message);
			}
		});
#endif
		return true;
	}
}