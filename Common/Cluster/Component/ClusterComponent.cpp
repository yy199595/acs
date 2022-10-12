//
// Created by zmhy0073 on 2022/10/12.
//

#include "ClusterComponent.h"
#include"App/App.h"
#include"File/FileHelper.h"
namespace Cluster
{
    Server::Server(const std::string &name)
        : mName(name), mIsAuthAllot(false) { }

    bool Server::LoadService(const rapidjson::Value &value)
    {
        LOG_CHECK_RET_FALSE(value.IsObject());
        LOG_CHECK_RET_FALSE(value.HasMember("Start"));
        LOG_CHECK_RET_FALSE(value.HasMember("AutoAllot"));

        this->mServices.clear();
        const rapidjson::Value &starts = value["Start"];
        this->mIsAuthAllot = value["AutoAllot"].GetBool();
        for (size_t index = 0; index < starts.Size(); index++)
        {
            this->mServices.emplace_back(std::string(starts[index].GetString()));
        }
        return true;
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
    void ClusterComponent::Awake()
    {
        std::string path;
        const ServerConfig & config = this->GetConfig();
        if(!config.GetPath("cluster", path))
        {
            THROW_LOGIC_ERROR("not find path 'cluster'");
        }
        rapidjson::Document document;
        if(!Helper::File::ReadJsonFile(path, document))
        {
            THROW_LOGIC_ERROR("read json file " + path + " error");
        }
        auto iter = document.MemberBegin();
        for(; iter != document.MemberEnd(); iter++)
        {
            const std::string name(iter->name.GetString());
            Cluster::Server * server = new Cluster::Server(name);
            if(!server->LoadService(iter->value))
            {
                THROW_LOGIC_ERROR("load service json : " << name);
            }
            this->mServers.emplace(name, server);
        }
        const std::string & name = config.GetNodeName();
        Cluster::Server * server = this->mServers[name];
        for(const std::string & service : server->GetServices())
        {

        }
    }
}