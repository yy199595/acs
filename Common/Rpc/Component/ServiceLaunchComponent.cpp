//
// Created by zmhy0073 on 2022/10/9.
//

#include"ServiceLaunchComponent.h"
#include"App/App.h"
#include"File/FileHelper.h"
#include"Service/LocalService.h"
#include"Service/LocalHttpService.h"
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
        if(!Helper::File::ReadJsonFile(path, jsonDocument))
        {
            return false;
        }
        auto iter = jsonDocument.MemberBegin();
        for(; iter != jsonDocument.MemberEnd(); iter++)
        {
            const rapidjson::Value &value = iter->value;
            const std::string name(iter->name.GetString());
            Component *component = ComponentFactory::CreateComponent(name);
            if (component == nullptr)
            {
                std::string type(value["Type"].GetString());
                component = ComponentFactory::CreateComponent(type);
            }
            if (component == nullptr || !this->GetApp()->AddComponent(name, component))
            {
                CONSOLE_LOG_ERROR("add " << name << " failure");
                return false;
            }
            IServiceBase *serviceBase = component->Cast<IServiceBase>();
            if(serviceBase == nullptr || !serviceBase->LoadConfig(value))
            {
                CONSOLE_LOG_ERROR("load service config error : " << name);
                return false;
            }
        }
        return true;
    }

    bool ServiceLaunchComponent::Start()
    {
        std::vector<IServiceBase *> components;
        this->GetApp()->GetComponents(components);
        const std::string name = this->GetConfig().GetNodeName();
        for(IServiceBase * component : components)
        {
            LocalService * localService = dynamic_cast<LocalService*>(component);
            LocalHttpService * localHttpService = dynamic_cast<LocalHttpService*>(component);
            if(localService != nullptr)
            {
                const RpcServiceConfig & config = localService->GetServiceConfig();
                if(config.GetServer() == "AllServer" || config.GetServer() == name)
                {
                    if(!localService->Start())
                    {
                        LOG_ERROR("start service [" << config.GetName() << "] faillure");
                        return false;
                    }
                }
            }
            else if(localHttpService != nullptr)
            {
                const HttpServiceConfig & config = localHttpService->GetServiceConfig();
            }
            CONSOLE_LOG_INFO("start service [" << name << "] successful");
        }
        return true;
    }
}