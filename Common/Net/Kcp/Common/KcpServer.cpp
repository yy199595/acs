//
// Created by 64658 on 2024/10/26.
//

#include "KcpServer.h"
#include "Entity/Actor/App.h"
#include "Util/Tools/String.h"
#include "Util/Tools/TimeHelper.h"
namespace kcp
{
	Server::Server(asio::io_context& io, kcp::Server::Component* component, unsigned short port)
			: mContext(io), mSocket(io, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)),
			mComponent(component), mTimer(io)
	{
		this->mTimer.expires_after(std::chrono::milliseconds(KCP_UPDATE_INTERVAL));
		this->mTimer.async_wait(std::bind(&Server::OnUpdate, this, std::placeholders::_1));
	}

	void Server::StartReceive()
	{
		this->mSocket.async_receive_from(asio::buffer(this->mRecvBuffer),
				this->mSenderPoint, [this](const asio::error_code& code, size_t size)
				{
					if (code.value() == Asio::OK)
					{
						this->OnReceive(size);
					}
					this->StartReceive();
				});
	}

	bool Server::Send(const std::string& addr, tcp::IProto* message)
	{
		asio::post(this->mContext, [this, addr, message]()
		{
			auto iter = this->mClients.find(addr);
			if(iter == this->mClients.end())
			{
				delete message;
				return;
			}
			iter->second->Send(message);
		});
		return true;
	}

	void Server::Update()
	{
		long long t = help::Time::NowMil();
		long long t1 = help::Time::NowSec();
		auto iter = this->mClients.begin();
		for(; iter != this->mClients.end();)
		{
			if(t1 - iter->second->GetLastTime() >= KCP_TIME_OUT)
			{
				CONSOLE_LOG_ERROR("remove kcp client=>{}", iter->second->GetAddress());
				iter = this->mClients.erase(iter++);
				continue;
			}
			iter->second->Update(t);
			iter++;
		}
	}

	kcp::Session* Server::GetSession(const std::string& address)
	{
		auto iter = this->mClients.find(address);
		if (iter != this->mClients.end())
		{
			return iter->second.get();
		}
		kcp::Session* kcpSession = nullptr;
		std::unique_ptr<kcp::Session> session
				= std::make_unique<kcp::Session>(this->mSocket, this->mSenderPoint);
		kcpSession = session.get();
		this->mClients.emplace(address, std::move(session));
		CONSOLE_LOG_DEBUG("kcp client:{} connect server", address)
		return kcpSession;
	}

	void Server::OnUpdate(const asio::error_code& code)
	{
		this->Update();
		this->mTimer.expires_after(std::chrono::milliseconds(KCP_UPDATE_INTERVAL));
		this->mTimer.async_wait(std::bind(&Server::OnUpdate, this, std::placeholders::_1));
	}

	void Server::OnReceive(size_t size)
	{
		const unsigned short port = this->mSenderPoint.port();
		std::string ip = this->mSenderPoint.address().to_string();
		const std::string address = fmt::format("{}:{}", ip, port);

		kcp::Session * kcpSession = this->GetSession(address);
		if(kcpSession == nullptr)
		{
			return;
		}
		int len = kcpSession->Decode(this->mRecvBuffer.data(), (int)size, this->mDecodeBuffer);
		if(len <= 0)
		{
			return;
		}
		rpc::Packet * rpcPacket = new rpc::Packet();
		{
			if(!rpcPacket->Decode(this->mDecodeBuffer.data(), len))
			{
				delete rpcPacket;
				CONSOLE_LOG_ERROR("{}", std::string(this->mRecvBuffer.data(), len))
				return;
			}
			rpcPacket->SetNet(rpc::Net::Kcp);
			rpcPacket->TempHead().Add(rpc::Header::kcp_addr, address);
		}
		asio::post(acs::App::GetContext(), [this, rpcPacket]() {
			this->mComponent->OnMessage(rpcPacket, nullptr);
		});
	}
}