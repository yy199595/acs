//
// Created by mac on 2021/11/28.
//

#include"OuterClient.h"
#include"XCode/XCode.h"
#include"Util/Tools/TimeHelper.h"
#include"Entity/Actor/App.h"
#include"Gate/Component/OuterNetComponent.h"

namespace rpc
{
	OuterClient::OuterClient(int id, Component * component)
		: Client(rpc::OuterBufferMaxSize), mSockId(id)
	{
		this->mMaxQps = 0;
		this->mMessage = nullptr;
		this->mLastRecvTime = 0;
		this->mComponent = component;
		this->mDecodeState = tcp::Decode::None;
	}

	void OuterClient::Stop(int code)
	{
		if(this->mSocket == nullptr)
		{
			return;
		}
		Asio::Socket & sock = this->mSocket->Get();
		const Asio::Executor & executor = sock.get_executor();
		asio::post(executor, [this, code] { this->CloseSocket(code); });
	}

	void OuterClient::StartReceive(tcp::Socket * socket, int second)
	{
		this->SetSocket(socket);
#ifdef ONLY_MAIN_THREAD
		this->ReadLength(rpc::RPC_PACK_HEAD_LEN, second);
#else
		Asio::Socket & sock = this->mSocket->Get();
		const Asio::Executor & executor = sock.get_executor();
		asio::post(executor, [this, second]
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
			Asio::Context & t = acs::App::GetContext();
			t.post([this] { this->mComponent->OnTimeout(this->mSockId); });
			return;
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
			default:
				return false;
		}
	}

    void OuterClient::OnReceiveMessage(std::istream & readStream, size_t size)
	{
		switch (this->mDecodeState)
		{
			case tcp::Decode::None:
			{
				tcp::Data::Read(readStream, this->mProtoHead);

				if(!CheckProtoHead(this->mProtoHead))
				{
					this->CloseSocket(XCode::UnKnowPacket);
					return;
				}

				if(this->mProtoHead.Len >= this->mMaxCount)
				{
					this->CloseSocket(XCode::NetBigDataShutdown);
					return;
				}
				this->mMessage = new rpc::Packet();
				{
					this->mMessage->Init(this->mProtoHead);
				}
				this->mDecodeState = tcp::Decode::MessageBody;
				this->ReadLength(this->mProtoHead.Len);
			}
				break;
			case tcp::Decode::MessageBody:
			{
				if (this->mMessage->OnRecvMessage(readStream, size) != 0)
				{
					this->CloseSocket(XCode::UnKnowPacket);
					return;
				}
				this->mDecodeState = tcp::Decode::Done;
				break;
			}
		}
		if(this->mDecodeState != tcp::Decode::Done)
		{
			return;
		}
		rpc::Packet * request = this->mMessage;
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
		this->mDecodeState = tcp::Decode::None;
		this->mLastRecvTime = help::Time::NowSec();
		this->ReadLength(rpc::RPC_PACK_HEAD_LEN);
	}

    void OuterClient::CloseSocket(int code)
	{
		if(this->mSockId == 0)
		{
			return;
		}
		this->StopTimer();
		this->mSocket->Close();
		int sockId = this->mSockId;
		rpc::Packet * message = nullptr;
		while(this->mSendMessages.Pop(message))
		{
			delete message;
			message = nullptr;
		}
		this->mSockId = 0;
		this->ClearSendStream();
		this->ClearRecvStream();
		Asio::Socket & socket1 = this->mSocket->Get();
		asio::post(socket1.get_executor(), [this, code, sockId]()
		{
#ifdef ONLY_MAIN_THREAD
			this->mComponent->OnCloseSocket(sockId, code);
#else
			Asio::Context & t = acs::App::GetContext();
			t.post([this, code, sockId] { this->mComponent->OnCloseSocket(sockId, code); });
#endif
		});

	}

	void OuterClient::OnSendMessage()
	{
		rpc::Packet * message = nullptr;
		if(this->mSendMessages.Pop(message))
		{
			delete message;
			message = nullptr;
		}
		if(this->mSendMessages.Front(message))
		{
			this->Write(*message);
		}
	}

	void OuterClient::OnSendMessage(const asio::error_code& code)
	{
#ifdef __DEBUG__
		//CONSOLE_LOG_ERROR(code.message());
		const std::string& address = this->mSocket->GetAddress();
		CONSOLE_LOG_ERROR("send {} outer message error", address);
#endif
		this->CloseSocket(XCode::SendMessageFail);
	}

	bool OuterClient::Send(rpc::Packet* message)
	{
		if(this->mSocket == nullptr)
		{
			return false;
		}
		LOG_CHECK_RET_FALSE(message);
#ifdef ONLY_MAIN_THREAD
		this->Write(*message);
#else
		Asio::Socket & sock = this->mSocket->Get();
		const Asio::Executor & executor = sock.get_executor();
		asio::post(executor, [this, message]
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
}