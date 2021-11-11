//
// Created by zmhy0073 on 2021/10/11.
//

#include "NodeMaperComponent.h"
#include <Service/NodeProxy.h>
#include <Service/NodeProxyComponent.h>
namespace GameKeeper
{
    bool NodeMaperComponent::Awake()
    {
        this->mNodeProxyComponent = this->GetComponent<NodeProxyComponent>();
        return true;
    }

    void NodeMaperComponent::AddService(NodeProxy * node)
    {
        std::vector<std::string> services;
        node->GetServicers(services);
        for(const std::string & name : services)
        {
            this->mServiceMappers.emplace(name, node->GetNodeUId());
        }
    }

    NodeProxy * NodeMaperComponent::GetService(const std::string & service)
    {
        auto iter = this->mServiceMappers.find(service);
        if(iter == this->mServiceMappers.end())
        {
            NodeProxy * node = this->mNodeProxyComponent->GetNodeByServiceName(service);
            if(node != nullptr)
            {
                this->AddService(node);
                return node;
            }
            return nullptr;
        }
        const int uid = iter->second;
        return this->mNodeProxyComponent->GetServiceNode(uid);
    }
}