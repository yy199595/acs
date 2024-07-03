//
// Created by zmhy0073 on 2021/12/1.
//

#include"Gate.h"
#include"Core/Event/IEvent.h"
#include"Entity/Actor/App.h"
#include"Core/System/System.h"
#include"Common/Service/Login.h"
#include"Gate/Client/OuterClient.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Server/Config/CodeConfig.h"
#include"Http/Component/HttpComponent.h"
#include"Gate/Component/OuterNetComponent.h"
namespace joke
{
    Gate::Gate()
    {
		this->mActorComponent = nullptr;
        this->mOuterComponent = nullptr;
    }
	bool Gate::Awake()
	{
		this->mApp->AddComponent<OuterNetComponent>();
		return true;
	}

    bool Gate::OnInit()
    {
		BIND_PLAYER_RPC_METHOD(Gate::Ping);
		BIND_PLAYER_RPC_METHOD(Gate::Login);
		BIND_PLAYER_RPC_METHOD(Gate::Logout);
		this->mActorComponent = this->mApp->ActorMgr();
		this->mOuterComponent = this->GetComponent<OuterNetComponent>();
		return true;
    }

	int Gate::Ping(long long userId)
	{
		if(!this->mActorComponent->HasPlayer(userId))
		{
			return XCode::NotFindUser;
		}
		return XCode::Ok;
	}

	int Gate::Login(const rpc::Packet & request)
	{
		int sockId = 0;
		long long userId = 0;
		std::string serverName;
		const std::string func("Login.OnLogin");
		const std::string & token = request.GetBody();
		LOG_DEBUG("player login token={}", request.GetBody());
		ClusterConfig::Inst()->GetServerName("Account", serverName);
		Server * loginServer = this->mActorComponent->Random(serverName);
		if(loginServer == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		LOG_ERROR_CHECK_ARGS(request.ConstHead().Get("sock", sockId));
		std::string path = fmt::format("/account/verify?token={}", token);
		std::unique_ptr<json::r::Document> response = std::make_unique<json::r::Document>();
		if(loginServer->Get(path, response.get())!= XCode::Ok )
		{
			return XCode::Failure;
		}
		std::unique_ptr<json::r::Value> jsonObject;
		if(!response->Get("data", jsonObject))
		{
			return XCode::Failure;
		}
		jsonObject->Get("player_id", userId);
		if(this->mActorComponent->HasPlayer(userId))
		{
			return XCode::NotFindUser;
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

		s2s::login::request message;
		message.set_user_id(userId);
		std::vector<int> targetServers;
		for(const NodeConfig * nodeConfig : configs)
		{
			const std::string & name = nodeConfig->GetName();
			Server * server = this->mActorComponent->Random(name);
			if(server == nullptr)
			{
				LOG_ERROR("allot server {0} fail", name);
				return XCode::AddressAllotFailure;
			}
			int serverId = server->GetSrvId();
			if(nodeConfig->HasService(ComponentFactory::GetName<class Login>()))
			{
				targetServers.emplace_back(serverId);
			}
			player->AddAddr(name, serverId);
			message.mutable_actors()->insert({name, serverId});
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

	void Gate::OnDisConnect(long long id)
	{
		if(Player * player = this->mActorComponent->GetPlayer(id))
		{
			player->Send("Gate.Logout");
		}
	}

	int Gate::Logout(long long userId)
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
				if (nodeConfig != nullptr && nodeConfig->HasService(ComponentFactory::GetName<class Login>()))
				{
					s2s::logout::request request;
					request.set_user_id(userId);
					server->Send("Login.OnLogout", request);
				}
			}
		}
		this->mActorComponent->DelActor(userId);
		return XCode::Ok;
	}
}