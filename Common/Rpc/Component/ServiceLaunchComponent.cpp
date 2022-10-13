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
    void ServiceLaunchComponent::Awake()
    {
        std::string path;
        const ServerConfig & config = this->GetConfig();
        if (!config.GetPath("service", path))
        {
            throw std::logic_error("not find service config");
        }

        rapidjson::Document jsonDocument;
        std::vector<std::string> services;
        if(config.GetServices(services) == 0 ||
            !Helper::File::ReadJsonFile(path, jsonDocument))
        {
            throw std::logic_error("not find start service");
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
                    throw std::logic_error("add " + name + " failure");
                }
                IServiceBase *serviceBase = component->Cast<IServiceBase>();
                if (serviceBase == nullptr || (!serviceBase->LoadConfig(value)))
                {
                    throw std::logic_error("load service config error : " + name);
                }
            }
        }
    }

    bool ServiceLaunchComponent::Start()
    {
        std::vector<std::string> startServices;
        this->GetConfig().GetServices(startServices, true);
        const std::string & location = this->GetConfig().GetLocalHost();
        for(const std::string & name : startServices)
        {
            IServiceBase * component = this->GetComponent<IServiceBase>(name);
            LocalService * localService = dynamic_cast<LocalService*>(component);
            LocalHttpService * localHttpService = dynamic_cast<LocalHttpService*>(component);
            if(component != nullptr && !component->Start())
            {
                LOG_ERROR("start service [" << name << "] faillure");
                return false;
            }
            if(localService != nullptr)
            {
                localService->AddLocation(location);
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