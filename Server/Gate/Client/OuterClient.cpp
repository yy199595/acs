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

	OuterClient::~OuterClient() noexcept
	{
		while(!this->mSendMessages.empty())
		{
			this->mSendMessages.pop();
		}
	}

	void OuterClient::Stop(int code)
	{
		if(this->mSocket == nullptr)
		{
			return;
		}
		Asio::Context & context = this->mSocket->GetContext();
		asio::post(context, [this, code] { this->CloseSocket(code); });
	}

	void OuterClient::StartReceive(tcp::Socket * socket, int second)
	{
		this->SetSocket(socket);
#ifdef ONLY_MAIN_THREAD
		this->ReadLength(rpc::RPC_PACK_HEAD_LEN, second);
#else
		asio::post(this->mSocket->GetContext(), [this, second]
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
			asio::post(t, [this] { this->mComponent->OnTimeout(this->mSockId); });
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
		if(size <= 0 || code.value() != Asio::OK)
		{
			return;
		}
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
		rpc::Packet * request = this->mMessage.release();
		{
			request->SetSockId(this->mSockId);
		}
#ifdef __DEBUG__
	request->TempHead().Add(rpc::Header::from_addr, this->mSocket->GetAddress());
#endif
#ifdef ONLY_MAIN_THREAD
		this->mComponent->OnMessage(request, nullptr);
#else
		Asio::Context & t = acs::App::GetContext();
		if(this->mSockId < 0)
		{
			CONSOLE_LOG_ERROR("=============")
		}
		asio::post(t, [this, request] { this->mComponent->OnMessage(request, nullptr); });
#endif
		this->mDecodeState = tcp::Decode::None;
		this->mLastRecvTime = help::Time::NowSec();
		this->ReadLength(rpc::RPC_PACK_HEAD_LEN);
	}

    void OuterClient::CloseSocket(int code)
	{
		if(!this->mSocket->IsOpen())
		{
			return;
		}
		this->StopTimer();
		this->mSocket->Close();
		while(!this->mSendMessages.empty())
		{
			this->mSendMessages.pop();
		}
//		this->ClearSendStream();
//		this->ClearRecvStream();
		this->mDecodeState = tcp::Decode::None;
		Asio::Context & context = this->mSocket->GetContext();
		asio::post(context, [this, code, sockId = this->mSockId]()
		{
#ifdef ONLY_MAIN_THREAD
			this->mComponent->OnCloseSocket(sockId, code);
#else
			Asio::Context & t = acs::App::GetContext();
			asio::post(t, [this, code, sockId] { this->mComponent->OnCloseSocket(sockId, code); });
#endif
		});
	}

	void OuterClient::OnSendMessage()
	{
		if(!this->mSendMessages.empty())
		{
			this->mSendMessages.pop();
			if(!this->mSendMessages.empty())
			{
				this->Write(*this->mSendMessages.front());
			}
		}
	}

	void OuterClient::OnSendMessage(const asio::error_code& code)
	{
		//this->CloseSocket(XCode::SendMessageFail);
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
		asio::post(this->mSocket->GetContext(), [this, message]
		{
			std::unique_ptr<rpc::Packet> sendMessage(message);
			this->mSendMessages.emplace(std::move(sendMessage));
			if(this->mSendMessages.size() == 1)
			{
				this->Write(*message);
			}
		});
#endif
		return true;
	}
}