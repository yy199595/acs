//
// Created by 64658 on 2024/10/26.
//

#include "KcpServer.h"
#include "Util/Tools/String.h"
#include "Util/Tools/TimeHelper.h"

namespace kcp
{
	Server::Server(asio::io_context& io, kcp::Server::Component* component, unsigned short port, Asio::Context& main)
			: mContext(io), mSocket(io, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)), mComponent(component),
			  mTimer(io), mMainContext(main), mTime(KCP_UPDATE_INTERVAL), mRecvBuf(mRecvBuffer, kcp::BUFFER_COUNT)
	{

	}

	void Server::Start()
	{
		this->StartTimer();
		this->StartReceive();
	}

	void Server::StartTimer()
	{
		this->mTimer.expires_after(this->mTime);
		this->mTimer.async_wait([this](auto&& PH1){ this->Update(); });
	}

	void Server::StartReceive()
	{
		auto callback = [this](const asio::error_code& code, size_t size)
		{
			if (code.value() == Asio::OK)
			{
				this->OnReceive(size);
			}
			else if (code == asio::error::operation_aborted)
			{
				return;
			}
			asio::post(this->mContext, [this] { this->StartReceive(); });
		};
		this->mSocket.async_receive_from(this->mRecvBuf, this->mSenderPoint, callback);
	}

	bool Server::Send(const std::string& addr, tcp::IProto* message)
	{
		asio::post(this->mContext, [this, addr, message]()
		{
			auto iter = this->mClients.find(addr);
			if (iter == this->mClients.end())
			{
				delete message;
				return;
			}
			iter->second->Send(message);
		});
		return true;
	}

	void Server::RemoveSession(const std::string& address)
	{
		asio::post(this->mContext, [this, address]()
		{
			auto iter = this->mClients.find(address);
			if (iter == this->mClients.end())
			{
				return;
			}
			this->mClients.erase(iter);
		});
	}

	void Server::Update()
	{
		long long t = help::Time::NowMil();
		long long t1 = help::Time::NowSec();
		auto iter = this->mClients.begin();
		while (iter != this->mClients.end())
		{
			if (t1 - iter->second->GetLastTime() >= KCP_TIME_OUT)
			{
				CONSOLE_LOG_ERROR("remove kcp client=>{}", iter->second->GetAddress());
				iter = this->mClients.erase(iter++);
				continue;
			}
			iter->second->Update(t);
			iter++;
		}
		this->StartTimer();
	}

	kcp::Session* Server::GetSession(const std::string& address)
	{
		auto iter = this->mClients.find(address);
		if (iter != this->mClients.end())
		{
			return iter->second.get();
		}
		kcp::Session* kcpSession = nullptr;
		std::unique_ptr<kcp::Session> session = std::make_unique<kcp::Session>(this->mSocket, this->mSenderPoint);
		{
			kcpSession = session.get();
			this->mClients.emplace(address, std::move(session));
		}
		CONSOLE_LOG_DEBUG("kcp client:{} connect server", address)
		return kcpSession;
	}

	void Server::OnReceive(size_t size)
	{
		const unsigned short port = this->mSenderPoint.port();
		std::string ip = this->mSenderPoint.address().to_string();
		const std::string address = fmt::format("{}:{}", ip, port);

		kcp::Session* kcpSession = this->GetSession(address);
		if (kcpSession == nullptr)
		{
			return;
		}
		int len = kcpSession->Decode(this->mRecvBuffer, (int)size, this->mDecodeBuffer);
		if (len <= 0)
		{
			return;
		}
		std::unique_ptr<rpc::Message> rpcPacket = std::make_unique<rpc::Message>();
		{
			if (!rpcPacket->Decode(this->mDecodeBuffer, len))
			{
				CONSOLE_LOG_ERROR("{}", std::string(this->mRecvBuffer, len))
				return;
			}
			rpcPacket->SetNet(rpc::Net::Kcp);
			rpcPacket->TempHead().Add(rpc::Header::from_addr, address);
		}
		asio::post(this->mMainContext, [this, request = rpcPacket.release()]
		{
			this->mComponent->OnMessage(request, nullptr);
		});
	}
}