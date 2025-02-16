#include"InnerTcpClient.h"

#include"XCode/XCode.h"
#include"Net/Rpc/Common/Rpc.h"
#include"Core/System/System.h"
#include"Core/Queue/Queue.h"

namespace rpc
{
	InnerTcpClient::InnerTcpClient(int id, Component* component, bool client, Asio::Context & io)
			: Client(rpc::InnerBufferMaxSize), mSockId(id), mComponent(component),
			  mIsClient(client), mDecodeStatus(tcp::Decode::None), mMainContext(io), mClose(false)
	{

	}

	InnerTcpClient::~InnerTcpClient() noexcept
	{
		while (!this->mSendMessages.empty())
		{
			delete this->mSendMessages.front();
			this->mSendMessages.pop();
		}
	}

	bool InnerTcpClient::Send(rpc::Message* message)
	{
		LOG_CHECK_RET_FALSE(message);
		if (message->GetType() == rpc::Type::Response)
		{
			assert(message->GetRpcId() > 0);
		}
#ifdef ONLY_MAIN_THREAD
		this->mSendMessages.emplace(message);
		if (this->mSendMessages.size() == 1)
		{
			this->Write(*message);
		}
#else
		Asio::Context & context = this->mSocket->GetContext();
		asio::post(context, [this, self = this->shared_from_this(), message]
		{
			this->AddToSendQueue(message);
		});
#endif
		return true;
	}

	void InnerTcpClient::OnSendMessage(size_t size)
	{
		if (!this->mSendMessages.empty())
		{
			rpc::Message* waitMessage = this->mSendMessages.front();
			{
				if (waitMessage->GetType() == rpc::Type::Request && waitMessage->GetRpcId() != 0)
				{
					int rpcId = waitMessage->GetRpcId();
					this->mWaitResMessages.emplace(rpcId, waitMessage);
				}
				else
				{
					delete waitMessage;
				}
				this->mSendMessages.pop();
			}
			if (!this->mSendMessages.empty())
			{
				this->Write(*this->mSendMessages.front());
			}
		}
	}

	void InnerTcpClient::OnConnect(const Asio::Code &code, int count)
	{
		if (code.value() == Asio::OK)
		{
			this->StopTimer();
			this->ClearSendStream();
			if (!this->mSendMessages.empty())
			{
				this->Write(*this->mSendMessages.front());
			}
			this->ReadLength(rpc::RPC_PACK_HEAD_LEN);
			this->mSocket->SetOption(tcp::OptionType::NoDelay, true);
			this->mSocket->SetOption(tcp::OptionType::KeepAlive, true);
			//CONSOLE_LOG_INFO("connect server [{}] ok", this->GetAddress());
			return;
		}
		this->CloseSocket(XCode::NetConnectFailure);
	}

	void InnerTcpClient::OnSendMessage(const Asio::Code& code)
	{
		if (code != asio::error::operation_aborted
			&& this->mIsClient && !this->mClose)
		{
			this->Connect(5);
			return;
		}
		this->CloseSocket(XCode::NetSendFailure);
	}

	void InnerTcpClient::CloseSocket()
	{
		if(this->mClose) {
			return;
		}
		this->mDecodeStatus = tcp::Decode::None;
		if (this->mComponent == nullptr)
		{
			auto iter = this->mWaitResMessages.begin();
			for (; iter != this->mWaitResMessages.end(); iter++)
			{
				delete iter->second;
			}
			this->mWaitResMessages.clear();
			while (!this->mSendMessages.empty())
			{
				delete this->mSendMessages.front();
				this->mSendMessages.pop();
			}
			return;
		}
		auto iter = this->mWaitResMessages.begin();
		for (; iter != this->mWaitResMessages.end(); iter++) //发出去没有返回的数据
		{
			this->mSendMessages.emplace(iter->second);
		}
		this->mWaitResMessages.clear();

		asio::post(this->mMainContext, [this, self = this->shared_from_this()]
		{
			while (!this->mSendMessages.empty())
			{
				rpc::Message* data = this->mSendMessages.front();
				this->mComponent->OnSendFailure(this->mSockId, data);
				this->mSendMessages.pop();
			}
		});

		this->mClose = true;
		this->mSocket->Close();
	}

	void InnerTcpClient::CloseSocket(int code)
	{
		if(!this->mClose)
		{
			this->CloseSocket();
			std::shared_ptr<Client> self = this->shared_from_this();
			asio::post(this->mMainContext, [self, this, code]()
			{
				this->mComponent->OnClientError(this->mSockId, code);
			});
		}
	}

	void InnerTcpClient::OnReadError(const Asio::Code& code)
	{
		if (code != asio::error::operation_aborted)
		{
//#ifdef __DEBUG__
//			const std::string& address = this->mSocket->GetAddress();
//			CONSOLE_LOG_ERROR("receive {} inner message error : {}", address, code.message());
//#endif
			this->CloseSocket(XCode::NetReadFailure);
		}
	}

	bool InnerTcpClient::MakeMessage(const rpc::ProtoHead& header)
	{
		if (header.Type != rpc::Type::Response)
		{
			this->mMessage = std::make_unique<rpc::Message>();
			return true;
		}

		int rpcId = header.RpcId;
		auto iter = this->mWaitResMessages.find(rpcId);
		if (iter == this->mWaitResMessages.end())
		{
			return false;
		}
		this->mMessage.reset(iter->second);
		this->mWaitResMessages.erase(iter);
		return true;
	}

	void InnerTcpClient::OnReceiveMessage(std::istream& readStream, size_t size, const Asio::Code& code)
	{
		if (size == 0 || code.value() != Asio::OK)
		{
			return;
		}
		switch (this->mDecodeStatus)
		{
			case tcp::Decode::None:
			{
				tcp::Data::Read(readStream, this->mProtoHead);

				if (this->mProtoHead.Len >= this->mMaxCount)
				{
					this->CloseSocket(XCode::NetBigDataShutdown);
					return;
				}
				if (!this->MakeMessage(this->mProtoHead))
				{
					this->CloseSocket(XCode::UnKnowPacket);
					return;
				}
				this->mMessage->Init(this->mProtoHead);
				this->mDecodeStatus = tcp::Decode::MessageBody;
				this->ReadLength(this->mProtoHead.Len);
				return;
			}
			case tcp::Decode::MessageBody:
			{
				if (this->mMessage->OnRecvMessage(readStream, size) != 0)
				{
					this->CloseSocket(XCode::UnKnowPacket);
					return;
				}
			}
		}
		do
		{
			if (this->mComponent == nullptr)
			{
				this->mMessage.reset();
				break;
			}
			switch(this->mMessage->GetType())
			{
				case rpc::Type::Ping:
				{
					rpc::Message* pingMessage = new rpc::Message();
					{
						pingMessage->SetType(rpc::Type::Ping);
						this->AddToSendQueue(pingMessage);
					}
					break;
				}
				case rpc::Type::Pong:
					break;
				default:
				{
					rpc::Message* request = this->mMessage.release();
					{
						this->mMessage = nullptr;
						request->SetSockId(this->mSockId);
					}
#ifdef __DEBUG__
					request->TempHead().Add(rpc::Header::from_addr, this->mSocket->GetAddress());
#endif
					asio::post(this->mMainContext, [this, request] { this->mComponent->OnMessage(request, nullptr); });
					break;
				}
			}

		} while (false);
		this->mDecodeStatus = tcp::Decode::None;
		Asio::Context& context = this->mSocket->GetContext();
		std::shared_ptr<Client> self = this->shared_from_this();
		asio::post(context, [this, self]() { this->ReadLength(rpc::RPC_PACK_HEAD_LEN); });
	}

	void InnerTcpClient::AddToSendQueue(rpc::Message* message)
	{
		this->mSendMessages.emplace(message);
		if(this->mSendMessages.size() == 1)
		{
			this->Write(*message);
		}
	}

	void InnerTcpClient::Close()
	{
		Asio::Context & context = this->mSocket->GetContext();
		std::shared_ptr<Client> self = this->shared_from_this();
		asio::post(context, [this, self] { this->CloseSocket(); });
	}

	void InnerTcpClient::StartReceive(tcp::Socket* socket)
	{
		this->SetSocket(socket);
		this->mSocket->SetOption(tcp::OptionType::NoDelay, true);
		this->mSocket->SetOption(tcp::OptionType::KeepAlive, true);
		this->ReadLength(rpc::RPC_PACK_HEAD_LEN);

		Asio::Context & context = this->mSocket->GetContext();
		std::shared_ptr<Client> self = this->shared_from_this();
		asio::post(context, [this, self]
		{
			this->mSocket->SetOption(tcp::OptionType::NoDelay, true);
			this->mSocket->SetOption(tcp::OptionType::KeepAlive, true);
			this->ReadLength(rpc::RPC_PACK_HEAD_LEN);
		});
	}
}