//
// Created by zmhy0073 on 2021/12/1.
//

#include"Gate.h"
#include"Entity/Actor/App.h"
#include"Common/Service/Login.h"
#include"Gate/Client/OuterNetTcpClient.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Server/Config/CodeConfig.h"
#include"Gate/Component/OuterNetComponent.h"
namespace Tendo
{
    Gate::Gate()
    {
		this->mIndex = 0;
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
		BIND_COMMON_RPC_METHOD(Gate::Ping);
		BIND_COMMON_RPC_METHOD(Gate::Enter);
		BIND_COMMON_RPC_METHOD(Gate::Login);
		BIND_COMMON_RPC_METHOD(Gate::Logout);
		this->mActorComponent = this->mApp->ActorMgr();
		const ServerConfig* config = ServerConfig::Inst();
		this->mOuterComponent = this->GetComponent<OuterNetComponent>();
		LOG_CHECK_RET_FALSE(config->GetLocation("rpc", this->mInnerAddress));
		LOG_CHECK_RET_FALSE(config->GetLocation("gate", this->mOuterAddress));
		return true;
    }

	int Gate::Ping(long long userId)
	{
		return XCode::Successful;
	}

	int Gate::Enter(const s2s::allot_request &request)
    {
		long long userId = request.user_id();
		const std::string & token = request.token();
		if(this->mTokens.find(token) != this->mTokens.end())
		{
			return XCode::Failure;
		}

		this->mTokens.emplace(token, userId);
		const std::string & server = this->GetServer();
		const std::string & address = this->mInnerAddress;
		if(this->mActorComponent->GetActor(userId) != nullptr) //玩家在线
		{
			return XCode::Failure;
		}
		return XCode::Successful;
    }
	int Gate::Login(const Msg::Packet& packet)
	{
		std::string address;
		if(!packet.ConstHead().Get("cli", address))
		{
			return XCode::CallArgsError;
		}
		const std::string & token = packet.GetBody();
		auto iter = this->mTokens.find(token);
		if(iter == this->mTokens.end())
		{
			return XCode::NotFindUser;
		}
		long long userId = iter->second;
		this->mTokens.erase(iter);
		int code = this->OnLogin(userId, token);
		if(code != XCode::Successful)
		{
			this->mActorComponent->DelActor(userId);
			return code;
		}
		this->mOuterComponent->BindClient(address, userId);
		LOG_INFO("user:" << userId << " login to gate successful");
		return XCode::Successful;
	}

	std::shared_ptr<class PlayerActor> Gate::NewPlayer(long long userId)
	{
		int actorId = (int)this->mApp->GetActorId();
		std::shared_ptr<PlayerActor> player = std::make_shared<PlayerActor>(userId, actorId);
		{
			player->AddAddr(this->GetServer(), actorId);
		}
		return player;
	}

	int Gate::OnLogin(long long userId, const std::string & token)
	{
		const std::string func("Login.OnLogin");
		std::vector<const NodeConfig *> configs;
		ClusterConfig::Inst()->GetNodeConfigs(configs);
		std::shared_ptr<PlayerActor> player = this->NewPlayer(userId);

		s2s::login::request message;
		message.set_user_id(userId);
		for(const NodeConfig * nodeConfig : configs)
		{
			if (!nodeConfig->IsAuthAllot())
			{
				continue;
			}
			const std::string & name = nodeConfig->GetName();
			ServerActor * targetActor = this->mActorComponent->Random(name);
			if(targetActor == nullptr)
			{
				return XCode::AddressAllotFailure;
			}
			int serverId = (int)targetActor->GetActorId();

			player->AddAddr(name, serverId);
			message.mutable_actors()->insert({name, serverId});
		}
		std::vector<int> actors;
		player->GetActors(actors);
		for(int actorId : actors)
		{
			ServerActor * targetServer = this->mActorComponent->GetServer(actorId);
			if(targetServer == nullptr)
			{
				return XCode::NotFoundActor;
			}
			int code = targetServer->Call(func, message);
			if(code != XCode::Successful)
			{
				const std::string& desc = CodeConfig::Inst()->GetDesc(code);
				LOG_ERROR("call " << targetServer->Name() <<  "] code = " << desc);
				return XCode::Failure;
			}
		}

		this->mActorComponent->AddPlayer(player);
		return XCode::Successful;
	}

	int Gate::Logout(long long userId)
	{
		this->mOuterComponent->StopClient(userId);
		Actor * player = this->mApp->ActorMgr()->GetActor(userId);
		if(player == nullptr)
		{
			return XCode::NotFindUser;
		}
		return XCode::Successful;
	}
}