//
// Created by mac on 2021/11/28.
//

#include"OuterNetMessageComponent.h"
#include"Client/OuterNetClient.h"
#include"Config/ServiceConfig.h"
#include"OuterNetComponent.h"
#include"Md5/MD5.h"
#include"Component/InnerNetComponent.h"
#include"Service/LocalRpcService.h"
#include"Component/ProtoComponent.h"
#include"Component/LocationComponent.h"
#include"Component/ForwardHelperComponent.h"
#include"Component/InnerNetMessageComponent.h"

#include"Config/ClusterConfig.h"
namespace Sentry
{

	bool OuterNetMessageComponent::LateAwake()
	{
		this->mTaskComponent = this->mApp->GetTaskComponent();
		this->mTimerComponent = this->mApp->GetTimerComponent();
		this->mLocationComponent = this->GetComponent<LocationComponent>();
        this->mForwardComponent = this->GetComponent<ForwardHelperComponent>();
        this->mInnerMessageComponent = this->GetComponent<InnerNetMessageComponent>();
		LOG_CHECK_RET_FALSE(this->mOutNetComponent = this->GetComponent<OuterNetComponent>());
		return true;
	}

    bool OuterNetMessageComponent::CreateToken(long long userId, std::string & token)
    {
        long long nowTime = Helper::Time::GetNowSecTime();
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
            auto iter1 = this->mClientAddressMap.find(userId);
            if(iter1 != this->mClientAddressMap.end())
            {
                this->mClientAddressMap.erase(iter1);
            }
            this->mUserAddressMap.erase(iter);
        }
    }

    XCode OuterNetMessageComponent::OnAuth(const std::string &address, std::shared_ptr<Rpc::Packet> message)
    {
        std::string token;
        LOG_RPC_CHECK_ARGS(message->GetHead().Get("token", token));
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
        std::unique_ptr<LocationUnit> locationUnit(new LocationUnit(userId, address));

        std::vector<std::string> services;
        std::vector<const NodeConfig *> nodeConfigs;
        ClusterConfig::Inst()->GetNodeConfigs(nodeConfigs);
        std::unordered_map<std::string, std::string> locations;
        for(const NodeConfig * nodeConfig : nodeConfigs)
        {
            services.clear();
            if (!nodeConfig->IsAuthAllot() || nodeConfig->GetServices(services) <= 0)
            {
                continue;
            }

            std::string location;
            for (const std::string &service: services)
            {
                const RpcServiceConfig *rpcServiceConfig = RpcConfig::Inst()->GetConfig(service);
                if (rpcServiceConfig == nullptr || !rpcServiceConfig->IsClient())
                {
                    continue;
                }
                if (location.empty() && !this->mLocationComponent->AllotLocation(service, location))
                {
                    return XCode::NetWorkError;
                }
                locations.emplace(service, location);
                locationUnit->Add(service, location);
            }
        }
        if(!this->mForwardComponent->OnAllot(userId, locations))
        {
            return XCode::NetWorkError;
        }
		this->mLocationComponent->AddLocationUnit(std::move(locationUnit));
        return XCode::Successful;
    }

	XCode OuterNetMessageComponent::OnRequest(const std::string & address, std::shared_ptr<Rpc::Packet> message)
	{
        auto iter = this->mUserAddressMap.find(address);
        if(iter == this->mUserAddressMap.end() || iter->second == 0)
        {
            return XCode::NotFindUser;
        }
		std::string fullName;
		const Rpc::Head& head = message->GetHead();
		LOG_RPC_CHECK_ARGS(head.Get("func", fullName));
		const RpcMethodConfig* methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
		if (methodConfig == nullptr || methodConfig->Type != "Client")
		{
			return XCode::CallFunctionNotExist;
		}
        std::string target;
        long long userId = iter->second;
        LocationUnit* locationUnit = this->mLocationComponent->GetLocationUnit(userId);
        if (locationUnit == nullptr || !locationUnit->Get(methodConfig->Service, target))
        {
            return XCode::NotFindUser;
        }
        RpcService * targetService = this->mApp->GetService(methodConfig->Service);
		if (targetService == nullptr)
		{
			CONSOLE_LOG_ERROR("userid=" << userId <<
										" call [" << methodConfig->Service << "] not find");
			return XCode::CallServiceNotFound;
		}
		message->GetHead().Add("id", userId);
		if(!this->mInnerMessageComponent->Send(target, message))
		{
			return XCode::NetWorkError;
		}
		return XCode::Successful;
	}

	XCode OuterNetMessageComponent::OnResponse(const std::string & address, std::shared_ptr<Rpc::Packet> message)
	{
        LOG_RPC_CHECK_ARGS(message->GetHead().Has("rpc"));
        if(message->GetCode(XCode::Failure) == XCode::NetActiveShutdown)
        {
            this->mOutNetComponent->StartClose(address);
            return XCode::NetActiveShutdown;
        }

        message->SetType(Tcp::Type::Response);
		if (!this->mOutNetComponent->SendData(address, message))
		{
            CONSOLE_LOG_ERROR("send message to client " << address << " error");
			return XCode::NetWorkError;
		}
        CONSOLE_LOG_DEBUG("send message to client " << address << " successful");
		return XCode::Successful;
	}
}