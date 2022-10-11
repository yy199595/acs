//
// Created by zmhy0073 on 2022/10/9.
//

#include"ServiceLaunchComponent.h"
#include"App/App.h"
#include"File/FileHelper.h"
namespace Sentry
{
    bool ServiceLaunchComponent::InitService()
    {
        std::string path;
        const ServerConfig & config = this->GetConfig();
        if (!config.GetPath("service", path))
        {
            CONSOLE_LOG_ERROR("not find service config");
            return false;
        }

        rapidjson::Document jsonDocument;
        std::vector<const ServiceConfig *> serviceConfigs;
        if(Helper::File::ReadJsonFile(path, jsonDocument)
           && config.GetServiceConfigs(serviceConfigs) > 0)
        {
            for (const ServiceConfig *config: serviceConfigs)
            {
                const std::string &name = config->Name;
                if (!jsonDocument.HasMember(name.c_str()))
                {
                    CONSOLE_LOG_FATAL("not find service config " << name);
                    return false;
                }
                Component *component = ComponentFactory::CreateComponent(name);
                if (component == nullptr)
                {
                    component = ComponentFactory::CreateComponent(config->Type);
                }
                if (component == nullptr || !this->GetApp()->AddComponent(name, component))
                {
                    CONSOLE_LOG_ERROR("add " << name << " failure");
                    return false;
                }
                rapidjson::Value &json = jsonDocument[name.c_str()];
                IServiceBase *serviceBase = component->Cast<IServiceBase>();
                if (serviceBase == nullptr || !serviceBase->LoadConfig(json))
                {
                    CONSOLE_LOG_ERROR("load " << name << " config error");
                    return false;
                }
            }
        }
        return true;
    }

    bool ServiceLaunchComponent::Start()
    {
        std::vector<const ServiceConfig *> serviceConfigs;
        this->GetConfig().GetServiceConfigs(serviceConfigs);
        for (const ServiceConfig *config: serviceConfigs)
        {
            const std::string &name = config->Name;
            IServiceBase * service = this->GetComponent<IServiceBase>(name);
            if(service == nullptr)
            {
                return false;
            }
            if(config->IsStart && !service->Start())
            {
                LOG_ERROR("start service [" << name << "] faillure");
                return false;
            }
            CONSOLE_LOG_INFO("start service [" << name << "] successful");
        }
        return true;
    }
}