//
// Created by zmhy0073 on 2021/10/11.
//

#include "NodeMaperComponent.h"
#include <Service/ServiceNode.h>
#include <Service/ServiceNodeComponent.h>
namespace Sentry
{
    bool NodeMaperComponent::Awake()
    {
        this->mServiceNodeComponent = this->GetComponent<ServiceNodeComponent>();
        return true;
    }

    void NodeMaperComponent::AddService(ServiceNode * node)
    {
        std::vector<std::string> services;
        node->GetServicers(services);
        for(const std::string & name : services)
        {
            this->mServiceMappers.emplace(name, node->GetNodeUId());
        }
    }

    ServiceNode * NodeMaperComponent::GetService(const std::string & service)
    {
        auto iter = this->mServiceMappers.find(service);
        if(iter == this->mServiceMappers.end())
        {
            ServiceNode * node = this->mServiceNodeComponent->GetNodeByServiceName(service);
            if(node != nullptr)
            {
                this->AddService(node);
                return node;
            }
            return nullptr;
        }
        const int uid = iter->second;
        return this->mServiceNodeComponent->GetServiceNode(uid);
    }
}