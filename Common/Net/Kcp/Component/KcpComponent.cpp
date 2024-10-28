//
// Created by 64658 on 2024/10/24.
//

#include "XCode/XCode.h"
#include "Util/Tools/String.h"
#include "Message/c2s/c2s.pb.h"
#include "KcpComponent.h"
#include "Rpc/Client/Message.h"
#include "Entity/Actor/App.h"
#include "Kcp/Common/KcpClient.h"
#include "Kcp/Common/KcpSession.h"
#include "Server/Config/CodeConfig.h"
#include "Server/Component/ThreadComponent.h"
#include "Rpc/Component/DispatchComponent.h"

namespace acs
{
	KcpComponent::KcpComponent()
		: ISender(rpc::Net::Kcp)
	{
		this->mPort = 0;
		this->mActor = nullptr;
		this->mThread = nullptr;
		this->mDispatch = nullptr;
	}

	bool KcpComponent::LateAwake()
	{
		try
		{
			std::unique_ptr<json::r::Value> jsonObject;
			std::unique_ptr<json::r::Value> jsonObject1;
			const ServerConfig & config = this->mApp->Config();
			if(config.Get("listen", jsonObject) && jsonObject->Get("kcp",jsonObject1))
			{
				LOG_CHECK_RET_FALSE(jsonObject1->Get("port",this->mPort));
				LOG_CHECK_RET_FALSE(this->mActor = this->GetComponent<ActorComponent>())
				LOG_CHECK_RET_FALSE(this->mThread = this->GetComponent<ThreadComponent>())
				LOG_CHECK_RET_FALSE(this->mDispatch = this->GetComponent<DispatchComponent>());
				Asio::Context & context = this->mThread->GetContext();
				{
					this->mKcpServer = std::make_unique<kcp::Server>(context, this, this->mPort);
				}
				this->mKcpServer->StartReceive();
				const std::string & host = config.Host();
				LOG_INFO("listen [kcp:{}] ok", this->mPort);
				return this->mApp->AddListen("kcp", fmt::format("{}:{}", host, this->mPort));
			}
			return true;
		}
		catch(std::exception & e)
		{
			LOG_ERROR("listen kcp:{} =>{}", this->mPort, e.what());
			return false;
		}
	}

	int KcpComponent::Send(int id, rpc::Packet* message)
	{
		kcp::IClient* udpClient = nullptr;
		if (message->GetType() == rpc::Type::Response)
		{
			std::string address;
			if (!message->TempHead().Del("udp", address))
			{
				return XCode::SendMessageFail;
			}
			udpClient = this->GetClient(address);
		}
		else
		{
			udpClient = this->GetClient(id);
		}
		if (udpClient == nullptr)
		{
			return XCode::SendMessageFail;
		}
		udpClient->Send(message);
		return XCode::Ok;
	}

	void KcpComponent::OnMessage(rpc::Packet* request, rpc::Packet* response)
	{
		std::string address;
		if(!request->TempHead().Get("udp", address))
		{
			delete request;
			return;
		}
		if(this->GetClient(address) == nullptr)
		{
			if(this->MakeSession(address) == nullptr)
			{
				delete request;
				return;
			}
			LOG_INFO("udp listen client:({}) ok", address);
		}
		int code = XCode::Ok;
		switch(request->GetType())
		{
			case rpc::Type::Request:
				code = this->OnRequest(request);
				break;
			case rpc::Type::Response:
				code = this->mDispatch->OnMessage(request);
				break;
			default:
				code = XCode::Failure;
				break;
		}
		if(code != XCode::Ok) { delete request; }
	}

	int KcpComponent::OnRequest(rpc::Packet* message)
	{
		int code = this->mDispatch->OnMessage(message);
		if (code != XCode::Ok)
		{
			const std::string& desc = CodeConfig::Inst()->GetDesc(code);
			LOG_ERROR("call {} code = {}", message->GetHead().GetStr("func"), desc);

			if (message->GetRpcId() == 0)
			{
				return XCode::Failure;
			}
			message->Body()->clear();
			message->SetType(rpc::Type::Response);
			message->GetHead().Add(rpc::Header::code, code);
			return this->Send(message->SockId(), message);
		}
		return XCode::Ok;
	}

	kcp::IClient* KcpComponent::GetClient(int id)
	{
		std::string address;
		if(!this->mActor->GetListen(id, "kcp", address))
		{
			return nullptr;
		}
		kcp::IClient * udpClient = this->GetClient(address);
		if (udpClient == nullptr)
		{
			udpClient = this->MakeClient(address);
		}
		return udpClient;
	}

	kcp::IClient* KcpComponent::GetClient(const std::string& address)
	{
		auto iter = this->mClients.find(address);
		if(iter == this->mClients.end())
		{
			return nullptr;
		}
		return iter->second.get();
	}

	kcp::IClient * KcpComponent::MakeClient(const std::string& address)
	{
		std::string ip;
		unsigned short port = 0;
		if(!help::Str::SplitAddr(address, ip, port))
		{
			return nullptr;
		}
		kcp::Client * udpClient = nullptr;
		Asio::Context & ctx = this->mThread->GetContext();
		asio::any_io_executor executor = ctx.get_executor();
		asio_udp::endpoint remote(asio::ip::make_address(address), port);
		std::unique_ptr<kcp::Client> client = std::make_unique<kcp::Client>(ctx, this, remote);
		{
			udpClient = client.get();
			this->mClients.emplace(address, std::move(client));
			asio::post(executor, [udpClient]() {udpClient->StartReceive(); });
		}
		return udpClient;
	}

	kcp::IClient* KcpComponent::MakeSession(const std::string& address)
	{
		std::string ip;
		unsigned short port = 0;
		if(!help::Str::SplitAddr(address, ip, port))
		{
			return nullptr;
		}
		kcp::Session * udpSession = nullptr;
		asio_udp::socket & socket = this->mKcpServer->Socket();
		asio_udp::endpoint remote(asio::ip::make_address(address), port);
		std::unique_ptr<kcp::Session> client = std::make_unique<kcp::Session>(socket, remote);
		{
			udpSession = client.get();
			this->mClients.emplace(address, std::move(client));
		}
		return udpSession;
	}
}