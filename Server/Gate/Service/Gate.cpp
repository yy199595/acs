//
// Created by zmhy0073 on 2021/12/1.
//

#include"Gate.h"
#include"Entity/Unit/App.h"
#include"Common/Service/User.h"
#include"Gate/Client/OuterNetTcpClient.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Server/Config/CodeConfig.h"
#include"Gate/Component/OuterNetComponent.h"
#include"Rpc/Component/LocationComponent.h"
#include"Entity/Component/PlayerMgrComponent.h"
namespace Tendo
{
    Gate::Gate()
    {
		this->mIndex = 0;
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
		BIND_COMMON_RPC_METHOD(Gate::Login);
		BIND_COMMON_RPC_METHOD(Gate::Logout);
		BIND_COMMON_RPC_METHOD(Gate::Allocation);
		const ServerConfig* config = ServerConfig::Inst();
		this->mOuterComponent = this->GetComponent<OuterNetComponent>();
		this->mPlayerComponent = this->GetComponent<PlayerMgrComponent>();
		LOG_CHECK_RET_FALSE(config->GetLocation("rpc", this->mInnerAddress));
		LOG_CHECK_RET_FALSE(config->GetLocation("gate", this->mOuterAddress));
		return true;
    }

	int Gate::Ping(long long userId)
	{
		return XCode::Successful;
	}

    void Gate::OnStop()
    {
		
    }

	int Gate::Allocation(const s2s::allot_request &request)
    {
		long long userId = request.user_id();
		const std::string & token = request.token();
		if(this->mTokens.find(token) != this->mTokens.end())
		{
			return XCode::Failure;
		}

		this->mTokens.emplace(token, userId);
		const std::string & server = this->GetServer();
		std::unique_ptr<Player> player = std::make_unique<Player>(userId);
		{
			player->GetAddr(this->GetServer(), this->mInnerAddress);
		}
		this->mPlayerComponent->AddPlayer(std::move(player));
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
			this->mPlayerComponent->DelPlayer(userId);
			return code;
		}
		this->mOuterComponent->BindClient(address, userId);
		LOG_INFO("user:" << userId << " login to gate successful");
		return XCode::Successful;
	}

	int Gate::OnLogin(long long userId, const std::string & token)
	{
		int targetId = 0;
		std::string address;
		const std::string func("User.Login");
		std::vector<const NodeConfig *> configs;
		ClusterConfig::Inst()->GetNodeConfigs(configs);
		Player * player = this->mPlayerComponent->GetPlayer(userId);
		for(const NodeConfig * nodeConfig : configs)
		{
			if (!nodeConfig->IsAuthAllot())
			{
				continue;
			}
			s2s::user::login message;
			message.set_user_id(userId);
			const std::string& server = nodeConfig->GetName();
			if(!this->mApp->GetAddr(server, targetId) || !this->mApp->GetAddr(targetId, address))
			{
				this->mPlayerComponent->DelPlayer(userId);
				return XCode::AddressAllotFailure;
			}
			player->GetAddr(server, targetId);
			int code = this->mApp->Call(address, func, message);
			if(code != XCode::Successful)
			{
				this->mPlayerComponent->DelPlayer(userId);
				const std::string& desc = CodeConfig::Inst()->GetDesc(code);
				LOG_ERROR("call " << server << " [" << address << "] code = " << desc);
				return XCode::Failure;
			}
			player->AddAddr(server, targetId);
			CONSOLE_LOG_INFO("add " << server << " [" << address << "] to " << userId);
		}
		return XCode::Successful;
	}

	int Gate::Logout(long long userId)
	{
		this->mOuterComponent->StopClient(userId);
		Player * player = this->mPlayerComponent->GetPlayer(userId);
		if(player == nullptr)
		{
			return XCode::NotFindUser;
		}
		player->BroadCast(std::string("User.Logout"));
		{
			this->mPlayerComponent->DelPlayer(userId);
			CONSOLE_LOG_ERROR("user:" << userId << " logout");
		}
		return XCode::Successful;
	}
}