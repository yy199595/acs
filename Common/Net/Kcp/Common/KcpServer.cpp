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
		asio::error_code code;
		this->mTimer.cancel(code);
		auto self = this->shared_from_this();
		this->mTimer.expires_after(this->mTime);
		this->mTimer.async_wait([self](auto&& PH1){ self->Update(); });
	}

	void Server::StartReceive()
	{
		auto self = this->shared_from_this();
		auto callback = [self](const asio::error_code& code, size_t size)
		{
			if (code.value() == Asio::OK)
			{
				self->OnReceive(size);
			}
			else if (code == asio::error::operation_aborted)
			{
				return;
			}
			asio::post(self->mContext, [self] { self->StartReceive(); });
		};
		this->mSocket.async_receive_from(this->mRecvBuf, this->mSenderPoint, callback);
	}

	bool Server::Send(const std::string& addr, tcp::IProto* message)
	{
		auto self = this->shared_from_this();
		asio::post(this->mContext, [this, addr, message, self]()
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
		auto self = this->shared_from_this();
		asio::post(this->mContext, [this, address, self]()
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
		for (auto iter = this->mClients.begin(); iter != this->mClients.end();)
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
		std::shared_ptr<kcp::Session> session = std::make_shared<kcp::Session>(this->mSocket, this->mSenderPoint);
		{
			this->mClients.emplace(address, session);
		}
		CONSOLE_LOG_DEBUG("kcp client:{} connect server", address)
		return session.get();
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
		this->mDecodeBuffer.resize(kcp::BUFFER_COUNT);
		char * buffer = (char*)this->mDecodeBuffer.data();
		int len = kcpSession->Decode(this->mRecvBuffer, (int)size, buffer);
		if (len <= 0)
		{
			return;
		}
		std::stringstream strBuffer(this->mDecodeBuffer);
		std::unique_ptr<rpc::Message> rpcPacket = std::make_unique<rpc::Message>();
		{
			rpcPacket->SetMsg(rpc::msg::text);
			tcp::Data::Read(strBuffer, rpcPacket->GetProtoHead());
			if (rpcPacket->OnRecvMessage(strBuffer, len) != 0)
			{
				CONSOLE_LOG_ERROR("{}", std::string(this->mRecvBuffer, len))
				return;
			}
			rpcPacket->SetNet(rpc::Net::Kcp);
			rpcPacket->TempHead().Add(rpc::Header::from_addr, address);
		}
		auto self = this->shared_from_this();
		asio::post(this->mMainContext, [this, self, request = rpcPacket.release()]
		{
			this->mComponent->OnMessage(request, nullptr);
		});
	}
}