//
// Created by zmhy0073 on 2022/10/13.
//

#include"TextConfigComponent.h"
#include"App/System/System.h"
#include"Config/ServerConfig.h"
#include"Config/ServiceConfig.h"
#include"Config/ClusterConfig.h"
#include"Config/CodeConfig.h"
#ifdef __ENABLE_REDIS__
#include"Config/RedisConfig.h"
#endif
#ifdef __ENABLE_MYSQL__
#include"Config/MysqlConfig.h"
#endif
#ifdef __ENABLE_MONGODB__
#include"Config/MongoConfig.h"
#endif
namespace Sentry
{
    bool TextConfigComponent::Awake()
    {
        if (!this->LoadTextConfig<ServerConfig>(System::GetConfigPath()))
        {
            return false;
        }
        std::string path;
        const ServerConfig *config = this->GetTextConfig<ServerConfig>();
        LOG_CHECK_RET_FALSE(config->GetPath("rpc", path));
        LOG_CHECK_RET_FALSE(this->LoadTextConfig<RpcConfig>(path));
        LOG_CHECK_RET_FALSE(config->GetPath("http", path));
        LOG_CHECK_RET_FALSE(this->LoadTextConfig<HttpConfig>(path));
        LOG_CHECK_RET_FALSE(config->GetPath("cluster", path));
        LOG_CHECK_RET_FALSE(this->LoadTextConfig<ClusterConfig>(path));
        LOG_CHECK_RET_FALSE(config->GetPath("code", path));
        LOG_CHECK_RET_FALSE(this->LoadTextConfig<CodeConfig>(path));
#ifdef __ENABLE_REDIS__
        LOG_CHECK_RET_FALSE(config->GetPath("redis", path));
        LOG_CHECK_RET_FALSE(this->LoadTextConfig<RedisConfig>(path));
#endif

#ifdef __ENABLE_MYSQL__
        LOG_CHECK_RET_FALSE(config->GetPath("mysql", path));
        LOG_CHECK_RET_FALSE(this->LoadTextConfig<MysqlConfig>(path));
#endif

#ifdef __ENABLE_MONGODB__
        LOG_CHECK_RET_FALSE(config->GetPath("mongo", path));
        LOG_CHECK_RET_FALSE(this->LoadTextConfig<MongoConfig>(path));
#endif
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
            else
            {
                const std::string & name = iter->first;
                LOG_INFO("reload [" << name << "] successful");
            }
        }
    }
}