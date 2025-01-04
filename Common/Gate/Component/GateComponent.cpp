//
// Created by leyi on 2023/9/11.
//
#include "XCode/XCode.h"
#include "GateComponent.h"
#include "Entity/Actor/App.h"
#include "Rpc/Config/MethodConfig.h"
#include "Entity/Component/ActorComponent.h"
#include "Router/Component/RouterComponent.h"
namespace acs
{
	GateComponent::GateComponent()
	{
		this->mActor = nullptr;
		this->mRouter = nullptr;
	}

	bool GateComponent::LateAwake()
	{
		std::vector<rpc::IOuterSender *> gateComponents;
		this->mApp->GetComponents(gateComponents);
		for(rpc::IOuterSender * gateComponent : gateComponents)
		{
			char net = gateComponent->GetNet();
			this->mOuterComponents.emplace(net, gateComponent);
		}
		LOG_CHECK_RET_FALSE(this->mActor = this->GetComponent<ActorComponent>())
		LOG_CHECK_RET_FALSE(this->mRouter = this->GetComponent<RouterComponent>())
		return true;
	}

	int GateComponent::Send(int id, rpc::Message* message)
	{
		char net = message->GetNet();
		auto iter = this->mOuterComponents.find(net);
		if(iter == this->mOuterComponents.end())
		{
			delete message;
			return XCode::NotFoundSender;
		}
		if(iter->second->Send(id, message) != XCode::Ok)
		{
			delete message;
			return XCode::SendMessageFail;
		}
		return XCode::Ok;
	}

	void GateComponent::Broadcast(rpc::Message* message)
	{

	}

	int GateComponent::OnMessage(rpc::Message* message)
	{
		int code = XCode::Failure;
		switch (message->GetType())
		{
			case rpc::Type::Request:
				code = this->OnRequest(message);
				break;
			case rpc::Type::Response:
				code = this->OnResponse(message);
				break;
			default:
				code = XCode::UnKnowPacket;
			LOG_ERROR("unknown message {}", message->ToString());
				break;
		}
		if (code != XCode::Ok)
		{
			delete message;
			return code;
		}
		return XCode::Ok;
	}

	int GateComponent::OnRequest(rpc::Message* message)
	{
		char net = message->GetNet();
		message->SetNet(rpc::Net::Tcp);
		message->GetHead().Add("n", net);
		message->SetSource(rpc::Source::Client);
		assert(message->GetHead().Has(rpc::Header::client_sock_id));
		const std::string& fullName = message->GetHead().GetStr(rpc::Header::func);
		const RpcMethodConfig* methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
		if (methodConfig == nullptr || !methodConfig->IsClient || !methodConfig->IsOpen)
		{
			LOG_ERROR("call function not exist : {}", fullName)
			return XCode::CallFunctionNotExist;
		}
		long long playerId = 0;
		message->SetNet(rpc::Net::Tcp);
		int serverId = this->mApp->GetSrvId();
		message->GetHead().Get(rpc::Header::player_id, playerId);
		Player* player = this->mActor->GetPlayer(playerId);
		if (player == nullptr)
		{
			if(methodConfig->IsAuth)
			{
				return XCode::CloseSocket;
			}
			this->mRouter->Send(serverId, std::unique_ptr<rpc::Message>(message));
			return XCode::Ok;
		}
		const std::string& name = methodConfig->Server;
		switch (methodConfig->Forward)
		{
			case rpc::Forward::Fixed: //转发到固定机器
				if (!player->GetServerId(name, serverId))
				{
					return XCode::NotFoundServerRpcAddress;
				}
				break;
			case rpc::Forward::Random:
			{
				Server* server = this->mActor->Random(name);
				if (server == nullptr)
				{
					return XCode::AddressAllotFailure;
				}
				if (!server->GetAddress(*message, serverId))
				{
					return XCode::NotFoundActor;
				}
				break;
			}
			case rpc::Forward::Hash:
			{
				Server* server = this->mActor->Hash(name, playerId);
				if (server == nullptr)
				{
					return XCode::AddressAllotFailure;
				}
				if (!server->GetAddress(*message, serverId))
				{
					return XCode::NotFoundActor;
				}
				break;
			}
			default:
			LOG_ERROR("unknown forward {}", message->ToString());
				return XCode::Failure;
		}
		this->mRouter->Send(serverId, std::unique_ptr<rpc::Message>(message));
		return XCode::Ok;
	}

	int GateComponent::OnResponse(rpc::Message* message)
	{
		int socketId, netType = 0;
		if (!message->GetHead().Del("n", netType))
		{
			return XCode::UnKnowPacket;
		}
		if (!message->GetHead().Del(rpc::Header::client_sock_id, socketId))
		{
			return XCode::UnKnowPacket;
		}
		message->SetNet((char)netType);
		message->SetSource(rpc::Source::Server);

		this->Send(socketId, message);
		return XCode::Ok;
	}
}