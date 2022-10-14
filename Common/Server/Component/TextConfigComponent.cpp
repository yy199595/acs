//
// Created by zmhy0073 on 2022/10/13.
//

#include"TextConfigComponent.h"
#include"App/System/System.h"
#include"Config/ServerConfig.h"
namespace Sentry
{
    bool TextConfigComponent::Awake()
    {
        if(!this->LoadTextConfig<ServerConfig>(System::GetConfigPath()))
        {
            return false;
        }
        return true;
    }

    void TextConfigComponent::OnHotFix()
    {
        for(auto & value : this->mConfigs)
        {
            if(!value.second->ReloadConfig())
            {
                CONSOLE_LOG_ERROR("reload [" << value.second->GetName() << "] failure");
            }
        }
    }
}