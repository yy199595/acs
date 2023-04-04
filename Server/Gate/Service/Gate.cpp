//
// Created by zmhy0073 on 2021/12/1.
//

#include"Gate.h"
#include"Entity/App/App.h"
#include"Util/Md5/MD5.h"
#include"Common/Service/User.h"
#include"Gate/Client/OuterNetClient.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Server/Config/CodeConfig.h"
#include"Gate/Component/OuterNetComponent.h"
#include"Rpc/Component/NodeMgrComponent.h"
namespace Tendo
{
    Gate::Gate()
    {
		this->mUserService = nullptr;
		this->mNodeComponent = nullptr;
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
		return true;
    }

	int Gate::Ping(long long userId)
	{
		return XCode::Successful;
	}

	bool Gate::OnStart()
	{
        const ServerConfig * config = ServerConfig::Inst();
		this->mUserService = this->mApp->GetService<User>();
		this->mNodeComponent = this->GetComponent<NodeMgrComponent>();
		this->mOuterComponent = this->GetComponent<OuterNetComponent>();
		LOG_CHECK_RET_FALSE(config->GetLocation("rpc", this->mInnerAddress));
		LOG_CHECK_RET_FALSE(config->GetLocation("gate", this->mOuterAddress));
        return true;
	}

    void Gate::OnClose()
    {
		this->GetComponent<OuterNetComponent>()->StopListen();
    }

	int Gate::Allocation(long long userId, s2s::allot::response &response)
    {
		long long now = Helper::Time::NowSecTime();
		std::string str = fmt::format("{0}:{1}", now, userId);
		const std::string token = Helper::Md5::GetMd5(str);
		if(this->mTokens.find(token) != this->mTokens.end())
		{
			return XCode::Failure;
		}
		this->mTokens.emplace(token, userId);
		{
			response.set_token(token);
			response.set_address(this->mOuterAddress);
		}
		const std::string & server = this->GetServer();
		this->mNodeComponent->AddRpcServer(server, userId, this->mInnerAddress);
		return XCode::Successful;
    }
	int Gate::Login(const Rpc::Packet& packet)
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
		int code = this->OnLogin(userId);
		if(code != XCode::Successful)
		{
			this->mNodeComponent->DelUnit(userId);
			return code;
		}
		this->mOuterComponent->BindClient(address, userId);
		LOG_INFO("user:" << userId << " login to gate successful");
		return XCode::Successful;
	}

	int Gate::OnLogin(long long userId)
	{
		static std::string func("Login");
		std::vector<const NodeConfig *> configs;
		ClusterConfig::Inst()->GetNodeConfigs(configs);
		for(const NodeConfig * nodeConfig : configs)
		{
			if (!nodeConfig->IsAuthAllot())
			{
				continue;
			}
			std::string address;
			s2s::user::login message;
			message.set_user_id(userId);
			const std::string& server = nodeConfig->GetName();
			if (!this->mNodeComponent->GetServer(server, address))
			{
				LOG_ERROR("user:" << userId << " allot [" << server << "]" << "error");
				return XCode::AddressAllotFailure;
			}
			int code = this->mUserService->Call(address, func, message);
			if(code != XCode::Successful)
			{
				const std::string& desc = CodeConfig::Inst()->GetDesc(code);
				LOG_ERROR("call " << server << " [" << address << "] code = " << desc);;
			}
			this->mNodeComponent->AddRpcServer(server, userId, address);
		}
		return XCode::Successful;
	}

	int Gate::Logout(long long userId)
	{
		this->mOuterComponent->StopClient(userId);
		std::unordered_map<std::string, std::string> userAddress;
		if(!this->mNodeComponent->GetServer(userId, userAddress))
		{
			return XCode::NotFindUser;
		}
		static std::string func("Logout");
		auto iter = userAddress.begin();
		for(; iter != userAddress.end(); iter++)
		{
			const std::string & address = iter->second;
			this->mUserService->Send(address, func, userId);
		}
		this->mNodeComponent->DelUnit(userId);
		CONSOLE_LOG_ERROR("user:" << userId << " logout");
		return XCode::Successful;
	}
}