//
// Created by mac on 2021/11/28.
//

#include"OuterNetMessageComponent.h"
#include"Client/OuterNetClient.h"
#include"Config/ServiceConfig.h"
#include"OuterNetComponent.h"
#include"Md5/MD5.h"
#include"Service/User.h"
#include"Component/InnerNetComponent.h"
#include"Service/PhysicalService.h"
#include"Component/ProtoComponent.h"
#include"Component/NodeMgrComponent.h"

#include"Component/InnerNetMessageComponent.h"

#include"Config/ClusterConfig.h"
namespace Sentry
{
    OuterNetMessageComponent::OuterNetMessageComponent()
    {
        this->mNodeComponent = nullptr;
        this->mInnerMessageComponent = nullptr;
    }


	bool OuterNetMessageComponent::LateAwake()
	{
		this->mNodeComponent = this->GetComponent<NodeMgrComponent>();
        this->mInnerMessageComponent = this->GetComponent<InnerNetMessageComponent>();
		return true;
	}

    bool OuterNetMessageComponent::CreateToken(long long userId, std::string & token)
    {
        long long nowTime = Helper::Time::NowSecTime();
        std::string rand = fmt::format("{0}:{1}", userId, nowTime);
        token = Helper::Md5::GetMd5(rand);
        this->mTokens.emplace(token, userId);
        return true;
    }

    bool OuterNetMessageComponent::GetAddress(long long id, std::string & address)
    {
        auto iter = this->mClientAddressMap.find(id);
        if(iter != this->mClientAddressMap.end())
        {
            address = iter->second;
            return true;
        }
        return false;
    }

    void OuterNetMessageComponent::OnClose(const std::string &address)
    {
        auto iter = this->mUserAddressMap.find(address);
        if(iter != this->mUserAddressMap.end())
        {
            long long userId = iter->second;
			this->OnClientLogout(userId);
            auto iter1 = this->mClientAddressMap.find(userId);
            if(iter1 != this->mClientAddressMap.end())
            {
                this->mClientAddressMap.erase(iter1);
            }
            this->mUserAddressMap.erase(iter);
        }
    }

    int OuterNetMessageComponent::OnAuth(const std::string &address, std::shared_ptr<Rpc::Packet> message)
    {
        std::string token;
        if(!message->GetHead().Get("token", token))
        {
            return XCode::CallArgsError;
        }
        if(this->mUserAddressMap.find(address) != this->mUserAddressMap.end())
        {
            return XCode::Successful;
        }
        auto iter = this->mTokens.find(token);
        if(iter == this->mTokens.end())
        {
            return XCode::Failure;
        }
        long long userId = iter->second;
        this->mUserAddressMap.emplace(address, userId);
        this->mClientAddressMap.emplace(userId, address);
		return this->OnClientLogin(userId);
    }

	int OuterNetMessageComponent::OnRequest(const std::string & address, std::shared_ptr<Rpc::Packet> message)
	{
        const Rpc::Head& head = message->GetHead();
        auto iter = this->mUserAddressMap.find(address);
        if(iter == this->mUserAddressMap.end() || iter->second == 0)
        {
            return XCode::NotFindUser;
        }
		std::string fullName;
		if(!head.Get("func", fullName))
        {
            return XCode::CallArgsError;
        }
		const RpcMethodConfig* methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
		if (methodConfig == nullptr || methodConfig->Type != "Client")
		{
			return XCode::CallFunctionNotExist;
		}
        if(!methodConfig->Request.empty() &&  message->GetSize() == 0)
        {
            return XCode::CallArgsError;
        }
        std::string target;
        long long userId = iter->second;
		const std::string & server = methodConfig->Server;
        if (!this->mNodeComponent->GetServer(server, userId, target))
        {
            return XCode::NotFindUser;
        }
        
		message->GetHead().Add("id", userId);
		if(!this->mInnerMessageComponent->Send(target, message))
		{
			return XCode::NetWorkError;
		}
		return XCode::Successful;
	}

	int OuterNetMessageComponent::OnClientLogin(long long int userId)
	{
		static std::string func("Login");
		std::vector<const NodeConfig *> configs;
		ClusterConfig::Inst()->GetNodeConfigs(configs);
		RpcService * rpcService = this->mApp->GetService<User>();
		for(const NodeConfig * nodeConfig : configs)
		{
			if(nodeConfig->IsAuthAllot())
			{
				std::string address;
				s2s::user::login message;
				const std::string & server = nodeConfig->GetName();
				if(!this->mNodeComponent->GetServer(server, address))
				{
					return XCode::AddressAllotFailure;
				}
				message.set_user_id(userId);
				rpcService->Send(address, func, message);
				this->mNodeComponent->AddRpcServer(server, userId, address);
			}
		}
		return XCode::Successful;
	}

	int OuterNetMessageComponent::OnClientLogout(long long int userId)
	{
		static std::string func("Logout");
		std::unordered_map<std::string, std::string> servers;
		if(!this->mNodeComponent->GetServer(userId, servers))
		{
			return XCode::Failure;
		}
		RpcService * rpcService = this->mApp->GetService<User>();
		if(rpcService == nullptr)
		{
			return XCode::CallServiceNotFound;
		}
		s2s::user::logout message;
		message.set_user_id(userId);
		for(const auto & info : servers)
		{
			const std::string & address = info.second;
			rpcService->Send(address, func, message);
		}
		return 0;
	}
}