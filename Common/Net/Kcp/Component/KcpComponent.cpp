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
#include "Rpc/Component/DispatchComponent.h"
#include "Server/Component/ThreadComponent.h"

namespace acs
{
	KcpComponent::KcpComponent()
		: ISender(rpc::Net::Kcp)
	{
		this->mPort = 0;
		this->mActor = nullptr;
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
				LOG_CHECK_RET_FALSE(this->mDispatch = this->GetComponent<DispatchComponent>());
				Asio::Context & context = this->GetComponent<ThreadComponent>()->GetContext();
				{
					this->mKcpServer = std::make_unique<kcp::Server>(context, this, this->mPort);
				}
				const std::string & host = config.Host();
				LOG_INFO("listen [kcp:{}] ok", this->mPort);
				asio::post(context, [this]() { this->mKcpServer->StartReceive(); });
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

	void KcpComponent::OnFrameUpdate()
	{
		long long t = help::Time::NowMil();
		auto iter = this->mClients.begin();
		for(; iter != this->mClients.end(); iter++)
		{
			iter->second->Update(t);
		}
	}

	int KcpComponent::Send(int id, rpc::Packet* message)
	{
		if (message->GetType() == rpc::Type::Response)
		{
			std::string address;
			if (!message->TempHead().Del(rpc::Header::kcp_addr, address))
			{
				return XCode::SendMessageFail;
			}
			if(this->mKcpServer->Send(address, message))
			{
				return XCode::Ok;
			}
			return XCode::SendMessageFail;
		}

		kcp::IClient* udpClient = this->GetClient(id);
		if (udpClient == nullptr)
		{
			return XCode::SendMessageFail;
		}
		udpClient->Send(message);
		return XCode::Ok;
	}

	void KcpComponent::OnMessage(rpc::Packet* request, rpc::Packet* response)
	{
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
		auto iter = this->mClients.find(id);
		if(iter != this->mClients.end())
		{
			return iter->second.get();
		}

		std::string address;
		if(!this->mActor->GetListen(id, "kcp", address))
		{
			return nullptr;
		}
		std::string ip;
		unsigned short port = 0;
		if(!help::Str::SplitAddr(address, ip, port))
		{
			return nullptr;
		}
		kcp::Client * udpClient = nullptr;
		Asio::Context & context = acs::App::GetContext();
		asio_udp::endpoint remote(asio::ip::make_address(ip), port);
		std::unique_ptr<kcp::Client> client = std::make_unique<kcp::Client>(context, this, remote);
		{
			client->StartReceive();
			udpClient = client.get();
			this->mClients.emplace(id, std::move(client));
		}
		return udpClient;
	}
}