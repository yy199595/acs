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
#include"Component/TranHelperComponent.h"
#include"Component/InnerNetMessageComponent.h"

#include"Config/ClusterConfig.h"
namespace Sentry
{
    OuterNetMessageComponent::OuterNetMessageComponent()
    {
		this->mSumCount = 0;
		this->mWaitCount = 0;
        this->mTranComponent = nullptr;
        this->mOutNetComponent = nullptr;
        this->mLocationComponent = nullptr;
        this->mInnerMessageComponent = nullptr;
    }


	bool OuterNetMessageComponent::LateAwake()
	{
		this->mLocationComponent = this->GetComponent<LocationComponent>();
        this->mTranComponent = this->GetComponent<TranHelperComponent>();
        this->mInnerMessageComponent = this->GetComponent<InnerNetMessageComponent>();
		LOG_CHECK_RET_FALSE(this->mOutNetComponent = this->GetComponent<OuterNetComponent>());
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
        std::unique_ptr<LocationUnit> locationUnit = std::make_unique<LocationUnit>(userId, address);

        std::vector<std::string> services;
        std::vector<const NodeConfig *> nodeConfigs;
        ClusterConfig::Inst()->GetNodeConfigs(nodeConfigs);
        for(const NodeConfig * nodeConfig : nodeConfigs)
        {
            services.clear();
            const std::string& server = nodeConfig->GetName();
            if (!nodeConfig->IsAuthAllot() || nodeConfig->GetServices(services) <= 0)
            {
                continue;
            }
            // ���������
            std::string location; 
            if (!this->mLocationComponent->AllotServer(server, location))
            {
                return XCode::NetWorkError;
            }
            locationUnit->Add(server, location);           
        }
        if(!this->mTranComponent->OnAllot(locationUnit.get()))
        {
            return XCode::NetWorkError;
        }
		this->mLocationComponent->AddUnit(std::move(locationUnit));
        return XCode::Successful;
    }

	XCode OuterNetMessageComponent::OnRequest(const std::string & address, std::shared_ptr<Rpc::Packet> message)
	{
		this->mSumCount++;
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
        std::string target, server;
        long long userId = iter->second;
        if (!ClusterConfig::Inst()->GetServerName(methodConfig->Service, server))
        {
            return XCode::CallServiceNotFound;
        }
        LocationUnit* locationUnit = this->mLocationComponent->GetUnit(userId);
        if (locationUnit == nullptr || !locationUnit->Get(server, target))
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

	XCode OuterNetMessageComponent::OnResponse(const std::string & address, std::shared_ptr<Rpc::Packet> message)
	{
		this->mWaitCount--;
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

	void OuterNetMessageComponent::OnRecord(Json::Writer& document)
	{
		document.Add("sum").Add(this->mSumCount);
		document.Add("wait").Add(this->mWaitCount);
	}
}