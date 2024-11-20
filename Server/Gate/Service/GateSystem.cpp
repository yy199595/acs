//
// Created by zmhy0073 on 2021/12/1.
//

#include"GateSystem.h"
#include"Entity/Actor/App.h"
#include"Core/System/System.h"
#include"Common/Service/LoginSystem.h"
#include"Gate/Client/OuterClient.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Server/Config/CodeConfig.h"
#include"Http/Component/HttpComponent.h"
#include "Timer/Component/TimerComponent.h"
#include"Gate/Component/OuterNetComponent.h"
namespace acs
{
    GateSystem::GateSystem()
    {
		this->mTimer = nullptr;
		this->mActorComponent = nullptr;
        this->mOuterComponent = nullptr;
    }
	bool GateSystem::Awake()
	{
		this->mApp->AddComponent<OuterNetComponent>();
		return true;
	}

    bool GateSystem::OnInit()
    {
		BIND_PLAYER_RPC_METHOD(GateSystem::Ping);
		BIND_PLAYER_RPC_METHOD(GateSystem::Login);
		BIND_PLAYER_RPC_METHOD(GateSystem::Logout);
		this->mTimer = this->GetComponent<TimerComponent>();
		this->mActorComponent = this->GetComponent<ActorComponent>();
		this->mOuterComponent = this->GetComponent<OuterNetComponent>();
		return true;
    }

	int GateSystem::Ping(long long userId)
	{
		if(!this->mActorComponent->HasPlayer(userId))
		{
			return XCode::NotFindUser;
		}
		return XCode::Ok;
	}

	int GateSystem::Login(const rpc::Packet & request)
	{
		int sockId = 0;
		request.ConstHead().Get("sock", sockId);
		const std::string & token = request.GetBody();
		const std::string func("LoginSystem.Login");

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
		int serverId = this->mApp->GetSrvId();
		Player * player = new Player(userId, serverId);
		{
			player->AddAddr(this->mApp->Name(), serverId);
			this->mOuterComponent->AddPlayer(userId, sockId);
			this->mActorComponent->AddPlayer(player);
		}

		std::vector<const NodeConfig *> configs;
		ClusterConfig::Inst()->GetNodeConfigs(configs);

		// TODO  给用户分配服务器

		s2s::login::request message;
		message.set_user_id(userId);
		std::vector<int> targetServers;
		for(const NodeConfig * nodeConfig : configs)
		{
			if (!nodeConfig->HasService(ComponentFactory::GetName<LoginSystem>()))
			{
				continue;
			}
			const std::string& name = nodeConfig->GetName();
			Server* server = this->mActorComponent->Random(name);
			if (server == nullptr)
			{
				LOG_ERROR("allot server [{0}] fail", name);
				return XCode::AddressAllotFailure;
			}
			int serverId = server->GetSrvId();
			targetServers.emplace_back(serverId);
			player->AddAddr(name, serverId);
			message.mutable_actors()->insert({ name, serverId });
		}

		for(const int & serverId : targetServers)
		{
			if(Server * server = this->mActorComponent->GetServer(serverId))
			{
				int code = server->Call(func, message);
				if(code != XCode::Ok)
				{
					LOG_ERROR("call {} code = {}", func, CodeConfig::Inst()->GetDesc(code));
					return XCode::Failure;
				}
			}
		}
		LOG_INFO("user:({}) login to gate successful", userId);
		return XCode::Ok;
	}

	void GateSystem::OnDisConnect(long long id)
	{
		if(Player * player = this->mActorComponent->GetPlayer(id))
		{
			player->Send("GateSystem.Logout");
		}
	}

	int GateSystem::Logout(long long userId)
	{
		this->mOuterComponent->StopClient(userId);
		Player * player = this->mActorComponent->GetPlayer(userId);
		if(player == nullptr)
		{
			return XCode::NotFindUser;
		}
		std::vector<int> servers;
		player->GetActors(servers);
		for (const int serverId : servers)
		{
			if (Server * server = this->mActorComponent->GetServer(serverId))
			{
				const std::string& name = server->Name();
				const NodeConfig * nodeConfig = ClusterConfig::Inst()->GetConfig(name);
				if (nodeConfig != nullptr && nodeConfig->HasService(ComponentFactory::GetName<class LoginSystem>()))
				{
					s2s::logout::request request;
					request.set_user_id(userId);
					server->Send("LoginSystem.Logout", request);
				}
			}
		}
		this->mActorComponent->DelActor(userId);
		LOG_WARN("user:({}) logout to gate successful", userId);
		return XCode::Ok;
	}
}