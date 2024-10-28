//
// Created by 64658 on 2024/10/24.
//

#include "XCode/XCode.h"
#include "Util/Tools/String.h"
#include "Message/c2s/c2s.pb.h"
#include "UdpComponent.h"
#include "Rpc/Client/Message.h"
#include "Entity/Actor/App.h"
#include "Net/Udp/Common/UdpClient.h"
#include "Server/Config/CodeConfig.h"
#include "Server/Component/ThreadComponent.h"
#include "Rpc/Component/DispatchComponent.h"

namespace acs
{
	UdpComponent::UdpComponent()
		: ISender(rpc::Net::Udp)
	{
		this->mPort = 0;
		this->mActor = nullptr;
		this->mThread = nullptr;
		this->mDispatch = nullptr;
	}

	bool UdpComponent::LateAwake()
	{
		try
		{
			std::unique_ptr<json::r::Value> jsonObject;
			std::unique_ptr<json::r::Value> jsonObject1;
			const ServerConfig & config = this->mApp->Config();
			if(config.Get("listen", jsonObject) && jsonObject->Get("udp",jsonObject1))
			{
				LOG_CHECK_RET_FALSE(jsonObject1->Get("port",this->mPort));
				LOG_CHECK_RET_FALSE(this->mActor = this->GetComponent<ActorComponent>())
				LOG_CHECK_RET_FALSE(this->mThread = this->GetComponent<ThreadComponent>())
				LOG_CHECK_RET_FALSE(this->mDispatch = this->GetComponent<DispatchComponent>());
				Asio::Context & context = this->mThread->GetContext();
				{
					this->mUdpServer = std::make_unique<udp::Server>(context, this, this->mPort);
				}
				this->mUdpServer->StartReceive();
				const std::string & host = config.Host();
				LOG_INFO("listen [udp:{}] ok", this->mPort);
				return this->mApp->AddListen("udp", fmt::format("{}:{}", host, this->mPort));
			}
			return true;
		}
		catch(std::exception & e)
		{
			LOG_ERROR("listen udp:{} =>{}", this->mPort, e.what());
			return false;
		}
	}

	int UdpComponent::Send(int id, rpc::Packet* message)
	{
		if (message->GetType() == rpc::Type::Response)
		{
			std::string address;
			if (!message->TempHead().Del("udp", address))
			{
				return XCode::SendMessageFail;
			}
			this->mUdpServer->Send(address, message);
			return XCode::Ok;
		}

		udp::IClient* udpClient = this->GetClient(id);
		if (udpClient == nullptr)
		{
			return XCode::SendMessageFail;
		}
		udpClient->Send(message);
		return XCode::Ok;
	}

	void UdpComponent::OnMessage(rpc::Packet* request, rpc::Packet* response)
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

	int UdpComponent::OnRequest(rpc::Packet* message)
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

	udp::Client* UdpComponent::GetClient(int id)
	{
		auto iter = this->mClients.find(id);
		if(iter != this->mClients.end())
		{
			return iter->second.get();
		}
		std::string address;
		if(!this->mActor->GetListen(id, "udp", address))
		{
			return nullptr;
		}
		std::string ip;
		unsigned short port = 0;
		if(!help::Str::SplitAddr(address, ip, port))
		{
			return nullptr;
		}
		udp::Client * udpClient = nullptr;
		Asio::Context & ctx = this->mThread->GetContext();
		asio::any_io_executor executor = ctx.get_executor();
		asio_udp::endpoint remote(asio::ip::make_address(ip), port);
		std::unique_ptr<udp::Client> client = std::make_unique<udp::Client>(ctx, this, remote);
		{
			udpClient = client.get();
			this->mClients.emplace(id, std::move(client));
			asio::post(executor, [udpClient]() {udpClient->StartReceive(); });
		}
		return udpClient;
	}
}