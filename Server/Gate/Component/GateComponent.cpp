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

	void GateComponent::BroadCast(const std::string& func, const pb::Message* data)
	{
		this->mGateServers.clear();
		this->mActor->GetServers(this->mGateName, this->mGateServers);

		for(const int & gateServerId : this->mGateServers)
		{
			if(Server * gate = this->mActor->GetServer(gateServerId))
			{
				std::unique_ptr<rpc::Packet> message = std::make_unique<rpc::Packet>();
				{
					message->SetType(rpc::Type::Broadcast);
					message->SetProto(rpc::Porto::Protobuf);
					message->GetHead().Add("func", "ChatComponent.OnChat");
					{
						message->WriteMessage(data);
						gate->SendMsg(std::move(message));
					}
				}
			}
		}
	}
}