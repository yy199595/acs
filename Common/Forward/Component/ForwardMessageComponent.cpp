//
// Created by zmhy0073 on 2022/10/19.
//
#include"Config/ClusterConfig.h"
#include"ForwardMessageComponent.h"
#include"Component/ForwardComponent.h"
#include"Component/LocationComponent.h"
namespace Sentry
{
    bool ForwardMessageComponent::LateAwake()
    {
        this->Add("Allot", &ForwardMessageComponent::Allot);
        this->Add("Remove", &ForwardMessageComponent::Remove);
        this->mForwardComponent = this->GetComponent<ForwardComponent>();
        this->mLocationComponent = this->GetComponent<LocationComponent>();
        return true;
    }

    bool ForwardMessageComponent::Add(const std::string &name, MessageCallback &&func)
    {
        if(this->mHandlers.find(name) == this->mHandlers.end())
        {
            return false;
        }
        this->mHandlers.emplace(name, std::move(func));
        return true;
    }

    XCode ForwardMessageComponent::OnMessage(std::shared_ptr<Rpc::Packet> message)
    {
        std::string func;
        if(!message->GetHead().Get("func", func))
        {
            return XCode::CallArgsError;
        }
        auto iter = this->mHandlers.find(func);
        if(iter == this->mHandlers.end())
        {
            return XCode::CallFunctionNotExist;
        }
        return (this->*iter->second)(message);
    }

    XCode ForwardMessageComponent::OnAuth(std::shared_ptr<Rpc::Packet> message)
    {
        std::string address;
        message->GetHead().Get("address", address);
        std::unique_ptr<ServiceNodeInfo> serverNode(new ServiceNodeInfo());
        {
            const Rpc::Head &head = message->GetHead();
            head.Get("rpc", serverNode->LocationRpc);
            head.Get("http", serverNode->LocationHttp);
            LOG_RPC_CHECK_ARGS(head.Get("name", serverNode->SrvName));
            LOG_RPC_CHECK_ARGS(head.Get("user", serverNode->UserName));
            LOG_RPC_CHECK_ARGS(head.Get("passwd", serverNode->PassWord));
            if (serverNode->LocationRpc.empty() && serverNode->LocationHttp.empty())
            {
                return XCode::CallArgsError;
            }
            if(ClusterConfig::Inst()->GetConfig(serverNode->SrvName) == nullptr)
            {
                LOG_ERROR("not find cluster config : " << serverNode->SrvName);
                return XCode::Failure;
            }
        }

        s2s::cluster::join request;
        request.set_name(serverNode->SrvName);
        request.set_rpc(serverNode->LocationRpc);
        request.set_http(serverNode->LocationHttp);
        this->mForwardComponent->Send("InnerService.Join", &request);

        auto iter = this->mNodeInfos.begin();
        for (; iter != this->mNodeInfos.end(); iter++)
        {
            request.Clear();
            request.set_name(iter->second->SrvName);
            request.set_rpc(iter->second->LocationRpc);
            request.set_http(iter->second->LocationHttp);
            this->mForwardComponent->Send(address, "InnerService.Join", &request);
        }
        this->mNodeInfos.emplace(address, std::move(serverNode));
        return XCode::Successful;
    }

    XCode ForwardMessageComponent::Allot(std::shared_ptr<Rpc::Packet> message)
    {
        long long userId = 0;
        std::vector<std::string> services;
        const Rpc::Head & head = message->GetHead();

        LOG_RPC_CHECK_ARGS(head.Get(services));
        LOG_RPC_CHECK_ARGS(head.Get("id", userId));
        LocationUnit * locationUnit = this->mLocationComponent->AddLocationUnit(userId);
        if(locationUnit == nullptr)
        {
            return XCode::Failure;
        }
        for(const std::string & service : services)
        {
            std::string address;
            if(head.Get(service, address))
            {
                locationUnit->Add(service, address);
            }
        }
        return XCode::Successful;
    }

    XCode ForwardMessageComponent::Remove(std::shared_ptr<Rpc::Packet> message)
    {
        long long userId = 0;
        if(!message->GetHead().Get("id", userId))
        {
            return XCode::CallArgsError;
        }
        this->mLocationComponent->DelLocationUnit(userId);
        return XCode::Successful;
    }
}