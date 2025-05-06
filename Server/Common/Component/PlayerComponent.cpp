//
// Created by 64658 on 2025/4/3.
//

#include "PlayerComponent.h"
#include "XCode/XCode.h"
#include "Rpc/Config/ServiceConfig.h"

namespace acs
{
	PlayerComponent::PlayerComponent()
	{

	}

	Player* PlayerComponent::Get(long long playerId)
	{
		if(playerId <= 0)
		{
			return nullptr;
		}
		auto iter = this->mPlayers.find(playerId);
		return iter != this->mPlayers.end() ? iter->second.get() : nullptr;
	}

	Actor* PlayerComponent::GetActor(long long playerId)
	{
		auto iter = this->mPlayers.find(playerId);
		return iter != this->mPlayers.end() ? iter->second.get() : nullptr;
	}

	int PlayerComponent::Broadcast(std::unique_ptr<rpc::Message> message, int & count)
	{
		count = 0;
		std::string func;
		if(!message->GetHead().Get(rpc::Header::func, func))
		{
			return XCode::NotFoundRpcConfig;
		}
		const RpcMethodConfig * rpcMethodConfig = RpcConfig::Inst()->GetMethodConfig(func);
		if(rpcMethodConfig == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		const std::string & server = rpcMethodConfig->server;
		for(auto iter = this->mPlayers.begin(); iter != this->mPlayers.end(); iter++)
		{
			int sockId = iter->second->GetClientID();
			std::unique_ptr<Player>& player = iter->second;
			if (!rpcMethodConfig->to_client && !player->GetServerId(server, sockId))
			{
				continue;
			}
			std::unique_ptr<rpc::Message> rpcMessage = message->Clone();
			{
				rpcMessage->SetSockId(sockId);
				rpcMessage->GetHead().Set(rpc::Header::id, std::to_string(iter->first));
				if (player->Send(std::move(rpcMessage)) == XCode::Ok)
				{
					++count;
				}
			}
		}
		return XCode::Ok;
	}

	bool PlayerComponent::Remove(long long playerId, bool notice)
	{
		auto iter = this->mPlayers.find(playerId);
		if(iter == this->mPlayers.end())
		{
			return false;
		}
		if(notice) {
			iter->second->Logout();
		}
		this->mPlayers.erase(iter);
		return true;
	}

	bool PlayerComponent::Add(std::unique_ptr<Player> player)
	{
		long long id = player->GetId();
		auto iter = this->mPlayers.find(id);
		if(iter != this->mPlayers.end())
		{
			return false;
		}
		if(!player->LateAwake())
		{
			return false;
		}
		this->mPlayers.emplace(id, std::move(player));
		return true;
	}
}