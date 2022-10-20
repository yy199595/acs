//
// Created by zmhy0073 on 2022/10/19.
//
#include"Config/ClusterConfig.h"
#include"Config/ServiceConfig.h"
#include"String/StringHelper.h"
#include"ForwardMessageComponent.h"
#include"Component/ForwardComponent.h"
#include"Component/LocationComponent.h"
namespace Sentry
{
    bool ForwardMessageComponent::LateAwake()
    {
        this->Add("Allot", &ForwardMessageComponent::Allot);
        this->Add("Remove", &ForwardMessageComponent::Remove);
        this->Add("Publish", &ForwardMessageComponent::Publish);
        this->Add("Subscribe", &ForwardMessageComponent::Subscribe);
        this->Add("UnSubscribe", &ForwardMessageComponent::UnSubscribe);
        LOG_CHECK_RET_FALSE(this->mForwardComponent = this->GetComponent<ForwardComponent>());
        LOG_CHECK_RET_FALSE(this->mLocationComponent = this->GetComponent<LocationComponent>());
        return true;
    }

    bool ForwardMessageComponent::Add(const std::string &name, MessageCallback &&func)
    {
        if(this->mHandlers.find(name) != this->mHandlers.end())
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

    const ServiceNodeInfo * ForwardMessageComponent::OnAuth(std::shared_ptr<Rpc::Packet> message)
    {
        std::string address;
        message->GetHead().Get("address", address);
        std::unique_ptr<ServiceNodeInfo> serverNode(new ServiceNodeInfo());
        {
            const Rpc::Head &head = message->GetHead();
            head.Get("rpc", serverNode->LocationRpc);
            head.Get("http", serverNode->LocationHttp);
            LOG_CHECK_RET_NULL(head.Get("name", serverNode->SrvName));
            LOG_CHECK_RET_NULL(head.Get("user", serverNode->UserName));
            LOG_CHECK_RET_NULL(head.Get("passwd", serverNode->PassWord));
            if (serverNode->LocationRpc.empty() && serverNode->LocationHttp.empty())
            {
                return nullptr;
            }
            if(ClusterConfig::Inst()->GetConfig(serverNode->SrvName) == nullptr)
            {
                LOG_ERROR("not find cluster config : " << serverNode->SrvName);
                return nullptr;
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
        return this->mNodeInfos[address].get();
    }

    XCode ForwardMessageComponent::Allot(std::shared_ptr<Rpc::Packet> message)
    {
        long long userId = 0;
        std::vector<std::string> services;
        const Rpc::Head & head = message->GetHead();
        {
            LOG_RPC_CHECK_ARGS(head.Get(services));
            LOG_RPC_CHECK_ARGS(head.Get("user_id", userId));
        }
        LocationUnit * locationUnit = this->mLocationComponent->AddLocationUnit(userId);
        if(locationUnit == nullptr)
        {
            return XCode::Failure;
        }
        for(const std::string & service : services)
        {
            std::string address;
            if(head.Get(service, address) &&
                RpcConfig::Inst()->GetConfig(service) != nullptr)
            {
                locationUnit->Add(service, address);
            }
        }
        return XCode::Successful;
    }

    XCode ForwardMessageComponent::Remove(std::shared_ptr<Rpc::Packet> message)
    {
        long long userId = 0;
        if(!message->GetHead().Get("user_id", userId))
        {
            return XCode::CallArgsError;
        }
        this->mLocationComponent->DelLocationUnit(userId);
        return XCode::Successful;
    }

    XCode ForwardMessageComponent::Publish(std::shared_ptr<Rpc::Packet> message)
    {
        int count = 0;
        std::string channel;
        const Rpc::Head & head = message->GetHead();
        LOG_RPC_CHECK_ARGS(head.Get("channel", channel));
        auto iter = this->mSubMessages.begin();
        for(; iter != this->mSubMessages.end(); iter++)
        {
            const std::string & address = iter->first;
            if(iter->second.find(channel) != iter->second.end())
            {
                count++;
                std::shared_ptr<Rpc::Packet> publish = std::make_shared<Rpc::Packet>();
                {
                    publish->SetProto(Tcp::Porto::String);
                    publish->SetType(Tcp::Type::SubPublish);
                    publish->SetContent(message->GetBody());
                    publish->GetHead().Add("channel", channel);
                }
                this->mForwardComponent->Send(address, publish);
            }
        }
        message->GetHead().Add("count", count);
        return XCode::Successful;
    }

    XCode ForwardMessageComponent::Subscribe(std::shared_ptr<Rpc::Packet> message)
    {
        std::string address;
        std::vector<std::string> channels;
        Rpc::Head & head = message->GetHead();
        LOG_RPC_CHECK_ARGS(head.Get("resp", address));
        if(Helper::String::Split(message->GetBody(), "/n", channels) <= 0)
        {
            return XCode::CallArgsError;
        }
        auto iter = this->mSubMessages.find(address);
        if(iter == this->mSubMessages.end())
        {
            std::set<std::string> subchannels;
            this->mSubMessages.emplace(address, subchannels);
        }
        int count = 0;
        std::set<std::string> & subchannels = this->mSubMessages[address];
        for(const std::string & channel : channels)
        {
            if(!channel.empty())
            {
                count++;
                subchannels.insert(channel);
            }
        }
        message->Clear();
        head.Add("count", count);
        return XCode::Successful;
    }

    XCode ForwardMessageComponent::UnSubscribe(std::shared_ptr<Rpc::Packet> message)
    {
        std::string address;
        std::vector<std::string> channels;
        Rpc::Head & head = message->GetHead();
        LOG_RPC_CHECK_ARGS(head.Get("resp", address));
        if(Helper::String::Split(message->GetBody(), "/n", channels) <= 0)
        {
            return XCode::CallArgsError;
        }
        auto iter = this->mSubMessages.find(address);
        if(iter == this->mSubMessages.end())
        {
            return XCode::Successful;
        }
        int count = 0;
        std::set<std::string> & subchannels = this->mSubMessages[address];
        for(const std::string & channel : channels)
        {
            auto iter = subchannels.find(channel);
            if(iter != subchannels.end())
            {
                count++;
                subchannels.erase(iter);
            }
        }
        head.Add("count", count);
        return XCode::Successful;
    }
}