//
// Created by zmhy0073 on 2022/10/9.
//

#include"ServiceLaunchComponent.h"
#include"App/App.h"
#include"File/FileHelper.h"
#include"Config/ServiceConfig.h"
#include"Service/LocalRpcService.h"
#include"Service/LocalHttpService.h"
#include"Component/LocationComponent.h"
namespace Sentry
{
    bool ServiceLaunchComponent::Awake()
    {
        std::string path;
        rapidjson::Document jsonDocument;
        std::vector<std::string> services;
        const ServerConfig * config = ServerConfig::Inst();
        LOG_CHECK_RET_FALSE(config->GetServices(services) > 0);
        LOG_CHECK_RET_FALSE(config->GetConfigPath("service", path));
        LOG_CHECK_RET_FALSE(Helper::File::ReadJsonFile(path, jsonDocument));

        for(const std::string & name : services)
        {
            Component *component = ComponentFactory::CreateComponent(name);
            const RpcServiceConfig * rpcServiceConfig = ServiceConfig::Inst()->GetRpcConfig(name);
            const HttpServiceConfig * httpServiceConfig = ServiceConfig::Inst()->GetHttpConfig(name);
            if(component == nullptr)
            {
                if(rpcServiceConfig != nullptr)
                {
                    const std::string & type = rpcServiceConfig->GetType();
                    component = ComponentFactory::CreateComponent(type);
                }
                else if(httpServiceConfig != nullptr)
                {
                    const std::string & type = httpServiceConfig->GetType();
                    component = ComponentFactory::CreateComponent(type);
                }
            }
            if (component == nullptr || !this->mApp->AddComponent(name, component))
            {
                CONSOLE_LOG_ERROR("add " << name << " failure");
                return false;
            }
        }
        return true;
    }

    bool ServiceLaunchComponent::Start()
    {
        std::string location;
        std::vector<std::string> startServices;
        const ServerConfig * config = ServerConfig::Inst();
        LOG_CHECK_RET_FALSE(config->GetLocation("rpc", location));
        LOG_CHECK_RET_FALSE(config->GetServices(startServices, true) > 0);
		LocationComponent * locationComponent = this->GetComponent<LocationComponent>();
        for(const std::string & name : startServices)
        {
            IServiceBase * component = this->GetComponent<IServiceBase>(name);
            LocalRpcService * localService = dynamic_cast<LocalRpcService*>(component);
            LocalHttpService * localHttpService = dynamic_cast<LocalHttpService*>(component);
            if(component != nullptr && !component->Start())
            {
                LOG_ERROR("start service [" << name << "] faillure");
                return false;
            }
            if(localService != nullptr)
            {
				locationComponent->AddLocation(name, location);
                CONSOLE_LOG_ERROR("start rpc service [" << name << "] successful");
            }
            else if(localHttpService != nullptr)
            {
                CONSOLE_LOG_ERROR("start http service [" << name << "] successful");
            }
        }
        return true;
    }
}