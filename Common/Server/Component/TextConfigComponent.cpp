//
// Created by zmhy0073 on 2022/10/13.
//

#include"TextConfigComponent.h"
#include"Core/System/System.h"
#include"Server/Config/ServerConfig.h"
#include"Server/Config/ServiceConfig.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Server/Config/CodeConfig.h"
namespace Sentry
{
    bool TextConfigComponent::Awake()
    {
        std::string path = System::ConfigPath();
        const ServerConfig* config = ServerConfig::Inst();
        if(config->GetPath("cluster", path))
        {
            LOG_CHECK_RET_FALSE(this->LoadTextConfig<ClusterConfig>(path));
        }
        if(config->GetPath("rpc", path))
        {
            LOG_CHECK_RET_FALSE(this->LoadTextConfig<RpcConfig>(path));
        }
        if(config->GetPath("http", path))
        {
            LOG_CHECK_RET_FALSE(this->LoadTextConfig<HttpConfig>(path));
        }

        if(config->GetPath("code", path))
        {
            LOG_CHECK_RET_FALSE(this->LoadTextConfig<CodeConfig>(path));
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
        //CONSOLE_LOG_INFO("load [" << name << "] successful path = " << path);
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
            else
            {
                const std::string & name = iter->first;
                LOG_INFO("reload [" << name << "] successful");
            }
        }
    }
}