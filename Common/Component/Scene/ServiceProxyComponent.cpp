#include"ServiceProxyComponent.h"
#include"RpcConfigComponent.h"
#include"Service/ServiceProxy.h"
namespace Sentry
{
    bool ServiceProxyComponent::Awake()
    {
        return true;
    }

    bool ServiceProxyComponent::LateAwake()
    {
        RpcConfigComponent * component = this->GetComponent<RpcConfigComponent>();
        if(component == nullptr)
        {
            return false;
        }
        std::vector<std::string> services;
        component->GetServices(services);
        for(const std::string & name : services)
        {
            std::shared_ptr<ServiceProxy> serviceEntity(new ServiceProxy(name));
            this->mServiceEntityMap.emplace(name, serviceEntity);
        }
        return true;
    }

    std::shared_ptr<ServiceProxy> ServiceProxyComponent::GetServiceProxy(const std::string &name)
    {
        auto iter = this->mServiceEntityMap.find(name);
        return iter != this->mServiceEntityMap.end() ? iter->second : nullptr;
    }
}// namespace Sentry
