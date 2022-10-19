//
// Created by zmhy0073 on 2022/10/19.
//

#include"ForwarMessageComponent.h"
#include"Component/ForwardComponent.h"
#include"Component/LocationComponent.h"
namespace Sentry
{
    bool ForwarMessageComponent::LateAwake()
    {
        this->Add("Allot", &ForwarMessageComponent::Allot);
        this->Add("Remove", &ForwarMessageComponent::Remove);
        this->mForwardComponent = this->GetComponent<ForwardComponent>();
        this->mLocationComponent = this->GetComponent<LocationComponent>();
        return true;
    }

    bool ForwarMessageComponent::Add(const std::string &name, MessageCallback &&func)
    {
        if(this->mHandlers.find(name) == this->mHandlers.end())
        {
            return false;
        }
        this->mHandlers.emplace(name, std::move(func));
        return true;
    }

    XCode ForwarMessageComponent::OnMessage(std::shared_ptr<Rpc::Packet> message)
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

    XCode ForwarMessageComponent::OnAuth(std::shared_ptr<Rpc::Packet> message)
    {
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
        }

        std::shared_ptr<Rpc::Packet> brocast =
            Rpc::Packet::New(Tcp::Type::Request, Tcp::Porto::Protobuf);
        {
            s2s::cluster::join request;
            request.set_name(serverNode->SrvName);
            request.set_rpc(serverNode->LocationRpc);
            request.set_http(serverNode->LocationHttp);

            brocast->WriteMessage(&request);
            brocast->GetHead().Add("func", "InnerService.Join");
            int count = this->mForwardComponent->Broadcast(brocast);
            CONSOLE_LOG_INFO(serverNode->SrvName << " join message broadcast " << count);
        }

        return XCode::Successful;
    }

    XCode ForwarMessageComponent::Allot(std::shared_ptr<Rpc::Packet> message)
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

    XCode ForwarMessageComponent::Remove(std::shared_ptr<Rpc::Packet> message)
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