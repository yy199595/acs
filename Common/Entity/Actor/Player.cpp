//
// Created by leyi on 2023/5/15.
//

#include"Player.h"
#include"XCode/XCode.h"
#include"Entity/Actor/App.h"
#include"Lua/Engine/Define.h"
#include"Server/Config/ServerConfig.h"
#include"Rpc/Component/InnerNetComponent.h"
#include "Util/Tools/TimeHelper.h"
#include "s2s/s2s.pb.h"

namespace acs
{
	Player::Player(long long playerId, int gate, int sockId)
		: Actor(playerId, "PLayer"), mGateId(gate), mSockId(sockId)
	{
		this->mActorComponent = App::ActorMgr();
	}

	bool Player::OnInit()
	{
		if(!this->mActorComponent->HasServer(this->mGateId))
		{
			LOG_ERROR("not find gate actor : {}", this->mGateId);
			return false;
		}
		return true;
	}

	void Player::Logout()
	{
		for (auto iter = this->mServerAddrs.begin(); iter != this->mServerAddrs.end(); iter++)
		{
			s2s::logout::request request;
			request.set_user_id(this->GetId());
			Server* server = this->mActorComponent->GetServer(iter->second);
			if (server != nullptr)
			{
				server->Send("LoginSystem.Logout", request);
			}
		}
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

	void Player::AddAddr(const std::string& server, int id)
	{
		if(server.empty())
		{
			return;
		}
		auto iter = this->mServerAddrs.find(server);
		if(iter == this->mServerAddrs.end() || iter->second != id)
		{
			this->mServerAddrs[server] = id;
			//LOG_DEBUG("player {} add [{}] id: {}", this->GetId(), server, id);
		}
	}

	void Player::GetActors(std::vector<int>& actors) const
	{
		actors.reserve(this->mServerAddrs.size());
		auto iter = this->mServerAddrs.begin();
		for(; iter != this->mServerAddrs.end(); iter++)
		{
			actors.emplace_back(iter->second);
		}
	}

	bool Player::GetServerId(const std::string& srv, int & id) const
	{
		auto iter = this->mServerAddrs.find(srv);
		if (iter == this->mServerAddrs.end())
		{
			return false;
		}
		id = iter->second;
		return true;
	}

	bool Player::GetAddress(const rpc::Message& request, int & id) const
	{
		const std::string & func = request.ConstHead().GetStr("func");
		const RpcMethodConfig * methodConfig = RpcConfig::Inst()->GetMethodConfig(func);
		return methodConfig != nullptr && this->GetServerId(methodConfig->Server, id);
	}

	void Player::EncodeToJson(std::string* json)
	{
		json::w::Document jsonWriter;
		jsonWriter.Add("id", this->GetId());
		std::unique_ptr<json::w::Value> data = jsonWriter.AddObject("addr");
		auto iter = this->mServerAddrs.begin();
		for(; iter != this->mServerAddrs.end(); iter++)
		{
			const std::string & name = iter->first;
			data->Add(name.c_str(), iter->second);
		}
		jsonWriter.Add("time", help::Time::NowSec());
		jsonWriter.Encode(json);
	}

	int Player::Make(const std::string& func, std::unique_ptr<rpc::Message>& message) const
	{
		const RpcMethodConfig * methodConfig = RpcConfig::Inst()->GetMethodConfig(func);
		if(methodConfig == nullptr)
		{
			LOG_ERROR("not rpc config {}", func);
			return XCode::NotFoundRpcConfig;
		}
		message = std::make_unique<rpc::Message>();
		{
			if(methodConfig->SendToClient)
			{
				message->SetSockId(this->mGateId);
				message->SetNet(methodConfig->Net);
				message->SetType(rpc::Type::Client);
				message->GetHead().Add(rpc::Header::client_sock_id, this->mSockId);
			}
			else
			{
				int serverId = 0;
				if(!this->GetServerId(methodConfig->Server, serverId))
				{
					LOG_ERROR("find {} id player:{} server:{}",
							methodConfig->Server, this->GetId(), methodConfig->Server);
					return XCode::AddressAllotFailure;
				}
				message->SetSockId(serverId);
				message->SetNet(methodConfig->Net);
				message->SetType(rpc::Type::Request);
			}
			message->SetProto(methodConfig->Proto);
			message->GetHead().Add(rpc::Header::func , func);
			message->GetHead().Add(rpc::Header::player_id, this->GetId());
		}
		message->SetTimeout(methodConfig->Timeout);
		return XCode::Ok;
	}
}