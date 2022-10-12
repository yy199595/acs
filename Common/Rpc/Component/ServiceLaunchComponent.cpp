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

        std::vector<std::string> services;
        if(config.GetServices(services) == 0)
        {
            return false;
        }
        rapidjson::Document jsonDocument;
        if(!Helper::File::ReadJsonFile(path, jsonDocument))
        {
            return false;
        }
        auto iter = jsonDocument.MemberBegin();
        for(; iter != jsonDocument.MemberEnd(); iter++)
        {
            const rapidjson::Value &value = iter->value;
            const std::string name(iter->name.GetString());
            if(std::find(services.begin(), services.end(), name) != services.end())
            {
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
                if (serviceBase == nullptr || (!serviceBase->LoadConfig(value)))
                {
                    CONSOLE_LOG_ERROR("load service config error : " << name);
                    return false;
                }
            }
        }
        return true;
    }

    bool ServiceLaunchComponent::Start()
    {
        std::vector<std::string> startServices;
        this->GetConfig().GetServices(startServices, true);
        for(const std::string & name : startServices)
        {
            IServiceBase * component = this->GetComponent<IServiceBase>(name);
            if(component != nullptr && !component->Start())
            {
                LOG_ERROR("start service [" << name << "] faillure");
                return false;
            }
        }
        return true;
    }
}