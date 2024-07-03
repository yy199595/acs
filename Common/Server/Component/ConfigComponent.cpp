//
// Created by zmhy0073 on 2022/10/13.
//

#include"ConfigComponent.h"
#include"Core/System/System.h"
#include"Http/Common/ContentType.h"
#include"Server/Config/ServerConfig.h"
#include"Rpc/Config/ServiceConfig.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Util/File/DirectoryHelper.h"
#include"Server/Config/CodeConfig.h"

#define LOAD_CONFIG_BY_PATH(cfg, name, T) 						\
{                                     						\
	std::string path;										\
	if(cfg->GetPath(name, path)) 							\
	{														\
		LOG_CHECK_RET_FALSE(this->LoadTextConfig<T>(path));	\
	}														\
}

namespace joke
{
    bool ConfigComponent::Awake()
	{
		std::string dir, path;
		if(ServerConfig::Inst()->GetPath("code", path))
		{
			LOG_CHECK_RET_FALSE(this->LoadTextConfig<CodeConfig>(path));
		}
		if(ServerConfig::Inst()->GetPath("cluster", path))
		{
			LOG_CHECK_RET_FALSE(this->LoadTextConfig<ClusterConfig>(path));
		}
		new http::ContentFactory();
		if (ServerConfig::Inst()->GetPath("rpc", dir))
		{
			std::vector<std::string> paths;
			std::unique_ptr<RpcConfig> config(new RpcConfig());
			help::dir::GetFilePaths(dir, "*.json", paths);
			for (const std::string& path: paths)
			{
				if(!config->LoadConfig(path))
				{
					LOG_ERROR("load [{}]", path);
					return false;
				}
			}
			this->mConfigs.emplace(config->GetConfigName(), std::move(config));
		}

		if (ServerConfig::Inst()->GetPath("http", dir))
		{
			std::vector<std::string> paths;
			std::unique_ptr<HttpConfig> config(new HttpConfig());
			help::dir::GetFilePaths(dir, "*.json", paths);

			for (const std::string& path: paths)
			{
				if (!config->LoadConfig(path))
				{
					LOG_ERROR("load http file : {}", path);
					return false;
				}
			}
			this->mConfigs.emplace(config->GetConfigName(), std::move(config));
		}

		return true;
	}

    bool ConfigComponent::LoadTextConfig(std::unique_ptr<ITextConfig> config, const std::string &path)
    {
        const std::string & name = config->GetConfigName();
        if(this->mConfigs.find(name) != this->mConfigs.end())
        {
            LOG_ERROR("multiple load {}", config->GetConfigName());
            return false;
        }
        if(!config->LoadConfig(path))
        {
            LOG_ERROR("load {} [{}] error", name, path);
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
                LOG_ERROR("reload {} fail", iter->second->Path());
				return false;
            }
        }
		return true;
    }
}