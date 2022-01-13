#include"ServiceComponent.h"
#include"RpcConfigComponent.h"
#include"Service/ServiceEntity.h"
namespace GameKeeper
{
    bool ServiceComponent::Awake()
    {
        return true;
    }

    bool ServiceComponent::LateAwake()
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
            std::shared_ptr<ServiceEntity> serviceEntity(new ServiceEntity(name));
            this->mServiceEntityMap.emplace(name, serviceEntity);
        }
        return true;
    }

    std::shared_ptr<ServiceEntity> ServiceComponent::GetServiceEntity(const std::string &name)
    {
        auto iter = this->mServiceEntityMap.find(name);
        return iter != this->mServiceEntityMap.end() ? iter->second : nullptr;
    }
}// namespace GameKeeper
