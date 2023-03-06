//
// Created by zmhy0073 on 2022/10/14.
//

#include"TranComponent.h"
#include"Helper/Helper.h"
#include"Service/RpcService.h"
#include"Config/ClusterConfig.h"
#include"Component/NodeMgrComponent.h"
#include"Component/NetThreadComponent.h"
#include"Component/InnerNetComponent.h"
namespace Sentry
{
    bool TranComponent::LateAwake()
    {
    
        return true;
    }

    int TranComponent::OnRequest(std::shared_ptr<Rpc::Packet> message)
    {
        std::string target;
        long long userId = 0;
        Rpc::Head &head = message->GetHead();
        if (head.Get("tran", userId)) //根据玩家id中转
        {
            head.Add("id", userId);
            return this->Forward(userId, message);
        }
        else if (head.Get("tran", target)) //根据地址中转
        {
            head.Remove("tran");
            return this->Forward(target, message);
        }
        return XCode::CallArgsError;
    }

    int TranComponent::Forward(long long userId, std::shared_ptr<Rpc::Packet> message)
    {
        std::string service, method, address, server;
        if(!message->GetMethod(service, method))
        {
            return XCode::CallArgsError;
        }
#ifdef __DEBUG__
        CONSOLE_LOG_DEBUG("forward message user id = "
            << userId << " func = " << service << "." << method);
#endif
        if (!ClusterConfig::Inst()->GetServerName(service, server))
        {
            return XCode::CallServiceNotFound;
        }
        LocationUnit *locationUnit = this->mLocationComponent->GetUnit(userId);
        if(locationUnit == nullptr)
        {
            return XCode::NotFindUser;
        }
        if (!locationUnit->Get(server, address))
        {
            return XCode::Failure;
        }
        return this->Forward(address, message);
    }

    int TranComponent::Forward(const std::string &address, std::shared_ptr<Rpc::Packet> message)
    {
#ifdef __DEBUG__
        std::string func;
        message->GetHead().Get("func", func);
        CONSOLE_LOG_DEBUG("forward message address = " << address << " func = " << func);
#endif
        if (!this->mInnerComponent->Send(address, message))
        {
            return XCode::NetWorkError;
        }      
        return XCode::Successful;
    }

	int TranComponent::OnResponse(std::shared_ptr<Rpc::Packet> message)
	{
		std::string address;
		if(!message->GetHead().Get("resp", address))
		{
            LOG_ERROR("not find head field : resp");
			return XCode::Failure;
		}
        message->GetHead().Remove("resp");
        message->SetType(Tcp::Type::Response);
        if (!this->mInnerComponent->Send(address, message))
        {
            return XCode::NetWorkError;
        }
        return XCode::Successful;		
	}
}