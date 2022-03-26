//
// Created by yjz on 2022/1/26.
//

#include"HttpNodeService.h"
#include"Object/App.h"
#include"Service/ServiceProxy.h"
#include"Component/Scene/ServiceMgrComponent.h"
namespace Sentry
{
    bool HttpNodeService::Awake()
    {
        BIND_HTTP_FUNCTION(HttpNodeService::Push);
        return true;
    }

    bool HttpNodeService::LateAwake()
    {
        this->mServiceComponent = this->GetComponent<ServiceMgrComponent>();
        return true;
    }

    XCode HttpNodeService::Push(const RapidJsonReader &jsonReader, RapidJsonWriter & response)
    {
        std::string address;
        std::vector<std::string> services;
        LOGIC_THROW_ERROR(jsonReader.TryGetValue("rpc", "address", address));
        LOGIC_THROW_ERROR(jsonReader.TryGetValue("rpc", "service", services));
        for(const std::string & service : services)
        {
            auto serviceProxy = this->mServiceComponent->GetServiceProxy(service);
            LOGIC_THROW_ERROR(serviceProxy);
            serviceProxy->AddAddress(address);
        }
        return XCode::Successful;
    }
}