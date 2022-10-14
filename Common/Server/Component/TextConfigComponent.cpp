//
// Created by zmhy0073 on 2022/10/13.
//

#include"TextConfigComponent.h"
#include"App/System/System.h"
#include"Config/ServerConfig.h"
#include"Config/ServiceConfig.h"
namespace Sentry
{
    bool TextConfigComponent::Awake()
    {
        if(!this->LoadTextConfig<ServerConfig>(System::GetConfigPath()))
        {
            return false;
        }
        std::string path;
        const ServerConfig * config = ServerConfig::Inst();
        if(config->GetConfigPath("service", path))
        {
            LOG_CHECK_RET_FALSE(this->LoadTextConfig<ServiceConfig>(path));
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
        for(auto & value : this->mConfigs)
        {
            if(!value.second->ReloadConfig())
            {
                LOG_ERROR("reload [" << value.second->GetName() << "] failure");
            }
        }
    }
}