//
// Created by leyi on 2023/5/15.
//

#include"Player.h"
#include"XCode/XCode.h"
#include"Entity/Actor/App.h"
#include"Server/Config/ServerConfig.h"
#include"Rpc/Component/InnerNetComponent.h"
#include"Rpc/Component/LocationComponent.h"
namespace Tendo
{
	Player::Player(long long playerId, const std::string & gate)
		: Actor(playerId, gate)
	{
		this->mActorComponent = App::Inst()->GetActorMgr();
	}
	bool Player::DelAddr(const std::string& server)
	{
		auto iter = this->mServerAddrs.find(server);
		if(iter == this->mServerAddrs.end())
		{
			return false;
		}
		this->mServerAddrs.erase(iter);
		return true;
	}

	void Player::AddAddr(const std::string& server, long long id)
	{
		this->mServerAddrs[server] = id;
	}

	int Player::SendToClient(const std::string& func)
	{
		std::shared_ptr<Msg::Packet> message = this->Make(func, nullptr);
		{
			message->SetType(Msg::Type::Client);
		}
		const std::string & gate = this->GetActorAddr();
		if(!this->mNetComponent->Send(gate, message))
		{
			return XCode::SendMessageFail;
		}
		return XCode::Successful;
	}

	int Player::SendToClient(const std::string& func, const pb::Message& request)
	{
		std::shared_ptr<Msg::Packet> message = this->Make(func, &request);
		{
			message->SetType(Msg::Type::Client);
		}
		const std::string & gate = this->GetActorAddr();
		if(!this->mNetComponent->Send(gate, message))
		{
			return XCode::SendMessageFail;
		}
		return XCode::Successful;
	}

	int Player::GetAddress(const std::string& func, std::string& addr)
	{
		const RpcMethodConfig * methodConfig = RpcConfig::Inst()->GetMethodConfig(func);
		if(methodConfig == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		auto iter = this->mServerAddrs.find(methodConfig->Server);
		if(iter  != this->mServerAddrs.end())
		{
			long long actorId = iter->second;
			Actor * targetActor = this->mActorComponent->GetActor(actorId);
			if(targetActor != nullptr)
			{
				addr = targetActor->GetActorAddr();
				return XCode::Successful;
			}
		}
		return XCode::NotFoundPlayerRpcAddress;
	}
}