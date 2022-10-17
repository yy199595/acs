//
// Created by zmhy0073 on 2022/10/13.
//

#include"TextConfigComponent.h"
#include"App/System/System.h"
#include"Config/ServerConfig.h"
#include"Config/ServiceConfig.h"
#include"Config/ClusterConfig.h"
namespace Sentry
{
    bool TextConfigComponent::Awake()
    {
        if(!this->LoadTextConfig<ServerConfig>(System::GetConfigPath()))
        {
            return false;
        }
        std::string path;
        const ServerConfig * config = this->GetTextConfig<ServerConfig>();
        if(config->GetConfigPath("rpc", path))
        {
            LOG_CHECK_RET_FALSE(this->LoadTextConfig<RpcConfig>(path));
        }
        if(config->GetConfigPath("http", path))
        {
            LOG_CHECK_RET_FALSE(this->LoadTextConfig<HttpConfig>(path));
        }
        if(config->GetConfigPath("cluster", path))
        {
            LOG_CHECK_RET_FALSE(this->LoadTextConfig<ClusterConfig>(path));
        }
        return true;
    }

    bool TextConfigComponent::LoadTextConfig(std::unique_ptr<TextConfig> config, const std::string &path)
    {
        const std::string & name = config->GetName();
        if(this->mConfigs.find(name) != this->mConfigs.end())
        {
            LOG_ERROR("multiple load [" << config->GetName() << "]");
            return false;
        }
        if(!config->LoadConfig(path))
        {
            LOG_ERROR("load [" << name << "] path:" << path << " error");
            return false;
        }
        this->mConfigs.emplace(name, std::move(config));
        CONSOLE_LOG_INFO("load [" << name << "] sucessful path = " << path);
        return true;
    }

    void TextConfigComponent::OnHotFix()
    {
        auto iter = this->mConfigs.begin();
        for(; iter != this->mConfigs.end(); iter++)
        {
            if(!iter->second->ReloadConfig())
            {
                const std::string & name = iter->first;
                LOG_ERROR("reload [" << name << "] failure");
            }
        }
    }
}