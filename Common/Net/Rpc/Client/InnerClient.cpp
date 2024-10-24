#include"InnerClient.h"

#include"XCode/XCode.h"
#include"Entity/Actor/App.h"
#include"Rpc/Client/Rpc.h"
#include"Core/System/System.h"
#include"Core/Queue/Queue.h"

namespace rpc
{
	InnerClient::InnerClient(int id, Component* component)
			: Client(rpc::InnerBufferMaxSize), mSockId(id), mComponent(component)
	{
		this->mMessage = nullptr;
		this->mDecodeStatus = tcp::Decode::None;
	}

	bool InnerClient::Send(rpc::Packet * message)
	{
		if (this->mSocket == nullptr)
		{
			return false;
		}
		LOG_CHECK_RET_FALSE(message);
		if(message->GetType() == rpc::Type::Response)
		{
			assert(message->GetRpcId() > 0);
		}
#ifdef ONLY_MAIN_THREAD
		this->mSendMessages.Push(message);
		if(this->mSendMessages.Size() == 1)
		{
			this->Write(*message);
		}
#else
		Asio::Socket & sock = this->mSocket->Get();
		asio::post(sock.get_executor(), [this, message]
		{
			this->mSendMessages.Push(message);
			if(this->mSendMessages.Size() == 1)
			{
				this->Write(*message);
			}
		});
#endif
		return true;
	}

	void InnerClient::OnSendMessage()
	{
		rpc::Packet * waitMessage = nullptr;
		if(this->mSendMessages.Pop(waitMessage))
		{
			if (waitMessage->GetType() ==
				rpc::Type::Request && waitMessage->GetRpcId() != 0)
			{
				int rpcId = waitMessage->GetRpcId();
				this->mWaitResMessages.Add(rpcId, waitMessage);
			}
			else
			{
				delete waitMessage;
				waitMessage = nullptr;
			}
			if(this->mSendMessages.Front(waitMessage))
			{
				this->Write(*waitMessage);
			}
		}
	}

	void InnerClient::OnConnect(bool result, int count)
	{
		if(result)
		{
			this->StopTimer();
			rpc::Packet * message;
			this->ClearSendStream();
			if(this->mSendMessages.Front(message))
			{
				this->Write(*message);
			}
			this->ReadLength(rpc::RPC_PACK_HEAD_LEN);
			this->mSocket->SetOption(tcp::OptionType::NoDelay, true);
			this->mSocket->SetOption(tcp::OptionType::KeepAlive, true);
			//CONSOLE_LOG_INFO("connect server [{}] ok", this->GetAddress());
			return;
		}
		this->CloseSocket(XCode::NetWorkError);
	}

	void InnerClient::OnSendMessage(const Asio::Code& code)
	{
		if (code == asio::error::operation_aborted)
		{
			return;
		}
		this->Connect(5);
	}

	void InnerClient::OnTimeout(tcp::TimeoutFlag flag)
	{
		switch(flag)
		{
			case tcp::TimeoutFlag::Write:
				this->CloseSocket(XCode::SendDataTimeout);
				break;
			case tcp::TimeoutFlag::Connect:
				this->CloseSocket(XCode::ConnectTimeout);
				break;
			case tcp::TimeoutFlag::ReadLine:
			case tcp::TimeoutFlag::ReadSome:
			case tcp::TimeoutFlag::ReadCount:
				this->CloseSocket(XCode::ReadDataTimeout);
				break;
		}
	}

	void InnerClient::CloseSocket(int code)
	{
		this->mDecodeStatus  = tcp::Decode::None;
		if (this->mComponent == nullptr)
		{
			this->mSendMessages.Clear();
			this->mWaitResMessages.Clear();
			return;
		}
		auto iter = this->mWaitResMessages.Begin();
		for (; iter != this->mWaitResMessages.End(); iter++) //发出去没有返回的数据
		{
			this->mSendMessages.Push(iter->second);
		}
		rpc::Packet * data = nullptr;
		this->mWaitResMessages.Clear();
		while (this->mSendMessages.Pop(data))
		{
#ifdef ONLY_MAIN_THREAD
			this->mComponent->OnSendFailure(this->mSockId, data);
#else
			Asio::Context & t = acs::App::GetContext();
			t.post([this, data] { this->mComponent->OnSendFailure(this->mSockId, data); });
#endif
		}

		if (!this->mSocket->IsClient())
		{
			this->mSocket->Close();
		}
		Asio::Socket & socket1 = this->mSocket->Get();
		asio::post(socket1.get_executor(), [this, code]()
		{
#ifdef ONLY_MAIN_THREAD
			this->mComponent->OnCloseSocket(this->mSockId, code);
#else
			Asio::Context & t = acs::App::GetContext();
			asio::post(t, [this, code] { this->mComponent->OnCloseSocket(this->mSockId, code); });
#endif
		});
	}

	void InnerClient::OnReadError(const Asio::Code& code)
	{
		if (code == asio::error::operation_aborted)
		{
			return;
		}
#ifdef __DEBUG__
		const std::string& address = this->mSocket->GetAddress();
		CONSOLE_LOG_ERROR("receive {} inner message error : {}", address, code.message());
#endif
		this->CloseSocket(XCode::NetReadFailure);
	}

	bool InnerClient::MakeMessage(const rpc::ProtoHead& header)
	{
		if (header.Type != rpc::Type::Response)
		{
			this->mMessage = std::make_unique<rpc::Packet>();
			return true;
		}

		int rpcId = header.RpcId;
		rpc::Packet* responseMessage = nullptr;
		if (!this->mWaitResMessages.Del(rpcId, responseMessage))
		{
			return false;
		}
		this->mMessage.reset(responseMessage);
		return true;
	}

	void InnerClient::OnReceiveMessage(std::istream& readStream, size_t size)
	{
		switch(this->mDecodeStatus)
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
			if(this->mComponent == nullptr)
			{
				this->mMessage.reset();
				break;
			}
			rpc::Packet * request = this->mMessage.release();
			{
				this->mMessage = nullptr;
				request->SetSockId(this->mSockId);
			}
#ifdef ONLY_MAIN_THREAD
			this->mComponent->OnMessage(request, nullptr);
#else
			Asio::Context & t = acs::App::GetContext();
			t.post([this, request] { this->mComponent->OnMessage(request, nullptr); });
#endif
		}
		while(false);
		this->mDecodeStatus = tcp::Decode::None;
#ifdef ONLY_MAIN_THREAD
		this->ReadLength(rpc::RPC_PACK_HEAD_LEN);
#else
		Asio::Socket & sock = this->mSocket->Get();
		const Asio::Executor & executor = sock.get_executor();
		asio::post(executor, [this] { this->ReadLength(rpc::RPC_PACK_HEAD_LEN); });
#endif
	}

	void InnerClient::Close()
	{
#ifdef ONLY_MAIN_THREAD
		this->CloseSocket(XCode::NetActiveShutdown);
#else
		Asio::Socket & sock = this->mSocket->Get();
		const Asio::Executor & executor = sock.get_executor();
		asio::post(executor, [this] { this->CloseSocket(XCode::NetActiveShutdown); });
#endif
	}

	void InnerClient::StartReceive(tcp::Socket * socket)
	{
		this->SetSocket(socket);
#ifdef ONLY_MAIN_THREAD
		this->mSocket->SetOption(tcp::OptionType::NoDelay, true);
		this->mSocket->SetOption(tcp::OptionType::KeepAlive, true);
		this->ReadLength(rpc::RPC_PACK_HEAD_LEN);
#else
		Asio::Socket & sock = this->mSocket->Get();
		const Asio::Executor & executor = sock.get_executor();
		asio::post(executor, [this]
		{
			this->mSocket->SetOption(tcp::OptionType::NoDelay, true);
			this->mSocket->SetOption(tcp::OptionType::KeepAlive, true);
			this->ReadLength(rpc::RPC_PACK_HEAD_LEN);
		});
#endif
	}
}