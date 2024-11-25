//
// Created by leyi on 2023/9/11.
//

#include"GateComponent.h"
#include"Gate/Service/GateSystem.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Entity/Component/ActorComponent.h"

namespace acs
{
	GateComponent::GateComponent()
	{
		this->mActor = nullptr;
	}

	bool GateComponent::LateAwake()
	{
		this->mActor = this->GetComponent<ActorComponent>();
		std::string name = ComponentFactory::GetName<GateSystem>();
		return ClusterConfig::Inst()->GetServerName(name, this->mGateName);
	}

	bool GateComponent::Send(long long playerId, const std::string& func, const pb::Message& message)
	{
		Player* player = this->mActor->GetPlayer(playerId);
		if (player == nullptr)
		{
			return false;
		}
		return player->Send(func, message) == XCode::Ok;
	}

	void GateComponent::BroadCast(const std::string& func, const pb::Message* data)
	{
		this->mGateServers.clear();
		if (this->mActor->GetServers(this->mGateName, this->mGateServers) <= 0)
		{
			return;
		}
		std::unique_ptr<rpc::Packet> message = std::make_unique<rpc::Packet>();
		{
			message->SetType(rpc::Type::Broadcast);
			message->SetProto(rpc::Porto::Protobuf);
			message->GetHead().Add("func", "ChatComponent.OnChat");
			{
				message->WriteMessage(data);
			}
		}
		for (const int& gateServerId: this->mGateServers)
		{
			if (Server* gate = this->mActor->GetServer(gateServerId))
			{
				gate->SendMsg(message->Clone());
			}
		}
	}
}