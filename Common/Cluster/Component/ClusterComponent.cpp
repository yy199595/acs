//
// Created by zmhy0073 on 2022/10/12.
//

#include "ClusterComponent.h"
#include"App/App.h"
#include"File/FileHelper.h"
#include"App/System/System.h"
#include"Service/LocalService.h"
#include"Service/LocalHttpService.h"
namespace Cluster
{
    Server::Server(const std::string &name)
        : mName(name), mIsAuthAllot(false) { }

    bool Server::LoadService(const rapidjson::Value &value)
    {
        LOG_CHECK_RET_FALSE(value.IsObject());
        LOG_CHECK_RET_FALSE(value.HasMember("Service"));
        LOG_CHECK_RET_FALSE(value.HasMember("AutoAllot"));

        this->mServices.clear();
        this->mIsAuthAllot = value["AutoAllot"].GetBool();
        const rapidjson::Value &services = value["Service"];
        for(auto iter = services.MemberBegin(); iter != services.MemberEnd(); iter++)
        {
            const std::string service(iter->name.GetString());
            this->mServices.emplace(service, iter->value.GetBool());
        }
        return true;
    }

    size_t Server::GetServices(std::vector<std::string> &services, bool start) const
    {
        for(auto & value : this->mServices)
        {
            if(start && !value.second)
            {
                continue;
            }
            services.emplace_back(value.first);
        }
        return services.size();
    }

    bool Server::LoadLocation(const rapidjson::Value &value)
    {
        LOG_CHECK_RET_FALSE(value.IsArray() && value.Size() > 0);
        this->mLocations.clear();
        for (size_t index = 0; index < value.Size(); index++)
        {
            this->mLocations.emplace_back(std::string(value[index].GetString()));
        }
        return true;
    }
}

namespace Sentry
{
    bool ClusterComponent::Awake()
    {
        std::string path;
        rapidjson::Document document;
        const ServerConfig * serverConfig = ServerConfig::Get();
        LOG_CHECK_RET_FALSE(serverConfig->GetConfigPath("cluster", path));
        LOG_CHECK_RET_FALSE(Helper::File::ReadJsonFile(path, document));
        auto iter = document.MemberBegin();
        for(; iter != document.MemberEnd(); iter++)
        {
            const std::string name(iter->name.GetString());
            std::unique_ptr<Cluster::Server> server(new Cluster::Server(name));
            if(!server->LoadService(iter->value))
            {
                CONSOLE_LOG_ERROR("load service json : " << name);
                return false;
            }
            this->mServers.emplace(name, std::move(server));
        }
        std::vector<std::string> services;
        const std::string & name = System::GetName();
        if(this->GetServer(name)->GetServices(services) > 0)
        {
            return this->CreateServices(services);
        }
        return true;
    }

    bool ClusterComponent::CreateServices(const std::vector<std::string> &services)
    {
        std::string path;
        rapidjson::Document jsonDocument;
        const ServerConfig * serverConfig = ServerConfig::Get();
        LOG_CHECK_RET_FALSE(serverConfig->GetConfigPath("service", path));
        LOG_CHECK_RET_FALSE(Helper::File::ReadJsonFile(path, jsonDocument));

        auto iter = jsonDocument.MemberBegin();
        for (; iter != jsonDocument.MemberEnd(); iter++)
        {
            const rapidjson::Value &value = iter->value;
            const std::string name(iter->name.GetString());
            if (std::find(services.begin(), services.end(), name) != services.end())
            {
                Component *component = ComponentFactory::CreateComponent(name);
                if (component == nullptr)
                {
                    std::string type(value["Type"].GetString());
                    component = ComponentFactory::CreateComponent(type);
                }
                if (component == nullptr || !this->GetApp()->AddComponent(name, component))
                {
                    CONSOLE_LOG_ERROR("add " + name + " failure");
                    return false;
                }
                IServiceBase *serviceBase = component->Cast<IServiceBase>();
                if (serviceBase == nullptr || (!serviceBase->LoadConfig(value)))
                {
                    CONSOLE_LOG_ERROR("load service config error : " + name);
                    return false;
                }
            }
        }
        return true;
    }

    bool ClusterComponent::Start()
    {
        std::vector<std::string> startServices;
        const std::string & name = System::GetName();
        const Cluster::Server * node = this->GetServer(name);
        if(node->GetServices(startServices, true) <= 0)
        {
            return false;
        }
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
                CONSOLE_LOG_ERROR("start rpc service [" << name << "] successful");
            }
            else if(localHttpService != nullptr)
            {
                CONSOLE_LOG_ERROR("start http service [" << name << "] successful");
            }
        }
        return true;
    }

    Cluster::Server *ClusterComponent::GetServer(const std::string &name)
    {
        auto iter = this->mServers.find(name);
        return iter != this->mServers.end() ? iter->second.get() : nullptr;
    }
}