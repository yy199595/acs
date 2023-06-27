//
// Created by zmhy0073 on 2022/10/13.
//

#include"ConfigComponent.h"
#include"Core/System/System.h"
#include"Server/Config/ServerConfig.h"
#include"Rpc/Config/ServiceConfig.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Server/Config/CodeConfig.h"
#include"Rpc/Config/ClientConfig.h"
#define TRY_LOAD_CONFIG(cfg, name, T) 						\
{                                     						\
	std::string path;										\
	if(cfg->GetPath(name, path)) 							\
	{														\
		LOG_CHECK_RET_FALSE(this->LoadTextConfig<T>(path));	\
	}														\
}
namespace Tendo
{
    bool ConfigComponent::Awake()
    {
        const ServerConfig* config = ServerConfig::Inst();
		{
			TRY_LOAD_CONFIG(config, "cluster", ClusterConfig);
			TRY_LOAD_CONFIG(config, "code", CodeConfig);

			TRY_LOAD_CONFIG(config, "http", HttpConfig);
			TRY_LOAD_CONFIG(config, "rpc", SrvRpcConfig);
			TRY_LOAD_CONFIG(config, "client", ClientConfig);
		}
		return true;
    }

    bool ConfigComponent::LoadTextConfig(std::unique_ptr<ITextConfig> config, const std::string &path)
    {
        const std::string & name = config->GetConfigName();
        if(this->mConfigs.find(name) != this->mConfigs.end())
        {
            LOG_ERROR("multiple load [" << config->GetConfigName() << "]");
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

    bool ConfigComponent::OnHotFix()
    {
        auto iter = this->mConfigs.begin();
        for(; iter != this->mConfigs.end(); iter++)
        {
            if(!iter->second->ReloadConfig())
            {
                const std::string & name = iter->first;
                LOG_ERROR("reload [" << name << "] failure");
				return false;
            }
        }
		return true;
    }
}