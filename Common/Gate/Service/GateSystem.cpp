//
// Created by zmhy0073 on 2021/12/1.
//

#include"GateSystem.h"
#include"Entity/Actor/App.h"
#include"Core/System/System.h"
#include"Core/Event/IEvent.h"
#include"Common/Service/LoginSystem.h"
#include"Cluster/Config/ClusterConfig.h"
#include "Util/Tools/TimeHelper.h"

namespace acs
{
    GateSystem::GateSystem()
    {
		this->mActor = nullptr;
    }

    bool GateSystem::OnInit()
    {
		BIND_PLAYER_RPC_METHOD(GateSystem::Ping);
		BIND_PLAYER_RPC_METHOD(GateSystem::Login);
		BIND_PLAYER_RPC_METHOD(GateSystem::Logout);
		BIND_PLAYER_RPC_METHOD(GateSystem::Create);
		this->mActor = this->GetComponent<ActorComponent>();
		return true;
    }

	int GateSystem::Ping(long long userId)
	{
		if(!this->mActor->HasPlayer(userId))
		{
			return XCode::NotFindUser;
		}
		return XCode::Ok;
	}

	bool GateSystem::AllotServer(std::vector<int>& servers)
	{
		std::vector<const NodeConfig *> configs;
		ClusterConfig::Inst()->GetNodeConfigs(configs);
		const std::string Name = ComponentFactory::GetName<LoginSystem>();

		for (const NodeConfig* nodeConfig: configs)
		{
			if (nodeConfig->HasService(Name))
			{
				const std::string& name = nodeConfig->GetName();
				Server* server = this->mActor->Random(name);
				if (server == nullptr)
				{
					LOG_ERROR("allot server [{0}] fail", name);
					return false;
				}
				servers.emplace_back(server->GetSrvId());
			}
		}
		return true;
	}

	int GateSystem::Login(const rpc::Message & request)
	{
		int sockId = 0;
		const rpc::Head & head = request.ConstHead();
		if(!head.Get(rpc::Header::client_sock_id, sockId))
		{
			return XCode::CallArgsError;
		}
		const std::string & token = request.GetBody();

		json::r::Document document;
		if(!this->mApp->DecodeSign(token, document))
		{
			return XCode::Failure;
		}
		long long userId, expTime = 0;
		long long nowTime = help::Time::NowSec();
		LOG_ERROR_CHECK_ARGS(document.Get("user_id", userId))
		LOG_ERROR_CHECK_ARGS(document.Get("end_time", expTime))
		if(nowTime >= expTime)
		{
			return XCode::TokenExpTime;
		}
		Player * oldPlayer = this->mActor->GetPlayer(userId);
		if(oldPlayer != nullptr)
		{
			return XCode::NotFindUser;
		}
		s2s::login::create message;
		{
			message.set_user_id(userId);
			message.set_sock_id(sockId);
		}
		return this->Create(message);
	}

	int GateSystem::Create(const s2s::login::create& request)
	{
		std::vector<int> servers;
		if(!this->AllotServer(servers))
		{
			return XCode::AddressAllotFailure;
		}
		int sockId = request.sock_id();
		long long userId = request.user_id();
		int serverId = this->mApp->GetSrvId();
		std::unique_ptr<Player> player = std::make_unique<Player>(userId, serverId, sockId);
		{
			s2s::login::request message;
			message.set_user_id(userId);
			message.set_client_id(sockId);
			player->AddAddr(this->mApp->Name(), serverId);
			s2s::server::info * serverInfo = message.add_list();
			{
				serverInfo->set_id(serverId);
				serverInfo->set_name(this->mApp->Name());
			}
			for(const int allotId : servers)
			{
				Server * server = this->mActor->GetServer(allotId);
				if(server == nullptr)
				{
					return XCode::NotFoundActor;
				}
				s2s::server::info * serverInfo = message.add_list();
				{
					serverInfo->set_name(server->Name());
					serverInfo->set_id(server->GetSrvId());
				}
			}

			for(const int allotId : servers)
			{
				Server * server = this->mActor->GetServer(allotId);
				if(server != nullptr && allotId != serverId)
				{
					if(server->Call("LoginSystem.Login", message) != XCode::Ok)
					{
						return XCode::Failure;
					}
					player->AddAddr(server->Name(), server->GetSrvId());
				}
			}
		}
		help::PlayerLoginEvent::Trigger(userId, sockId);
		this->mActor->AddPlayer(std::move(player));
		//LOG_INFO("user:({}) login to gate successful", userId);
		return XCode::Ok;
	}

	int GateSystem::Logout(const rpc::Message & request)
	{
		int sockId = 0;
		long long userId = 0;
		const rpc::Head & head = request.ConstHead();
		if(!head.Get(rpc::Header::client_sock_id, sockId))
		{
			LOG_ERROR("not find client_sock_id");
			return XCode::CallArgsError;
		}
		head.Get(rpc::Header::player_id, userId);
		Player * player = this->mActor->GetPlayer(userId);
		if(player == nullptr)
		{
			LOG_ERROR("not find player_id:{}", userId);
			return XCode::NotFindUser;
		}
		return XCode::CloseSocket;
	}
}