//
// Created by zmhy0073 on 2021/11/11.
//

#include "HttpOperComponent.h"
#include"Core/App.h"
#include"Scene/LoggerComponent.h"
namespace GameKeeper
{
    bool HttpOperComponent::Awake()
    {
        this->Add("Hotfix", &HttpOperComponent::Hotfix, this);
        this->Add("LoadConfig", &HttpOperComponent::LoadConfig, this);
        this->mOperAccountMap["root"] = "199595yjz";
        return true;
    }

    bool HttpOperComponent::LateAwake()
    {
        return true;
    }

    XCode HttpOperComponent::Hotfix(RapidJsonWriter & response)
    {
        std::vector<Component *> components;
        this->GetComponents(components);
        response.StartArray("components");
        for(Component * component : components)
        {
            if(auto hotfix = dynamic_cast<IHotfix*>(component))
            {
                hotfix->OnHotFix();
                response.Add(component->GetTypeName().c_str());
                LOG_DEBUG("========== " << component->GetTypeName() << " hotfix ==========");
            }
        }
        response.EndArray();
        return XCode::Successful;
    }

    XCode HttpOperComponent::LoadConfig(RapidJsonWriter & response)
    {
        response.StartArray("components");
        std::vector<Component *> allComponent;
        this->GetComponents(allComponent);
        for (Component *component: allComponent)
        {
            if (auto configModule = dynamic_cast<ILoadConfig *>(component))
            {
                configModule->OnLoadConfig();
                response.Add(component->GetTypeName().c_str());
                LOG_DEBUG(component->GetTypeName() << " load config");
            }
        }
        response.EndArray();
        return XCode::Successful;
    }

    XCode HttpOperComponent::VerifyAccount(const RapidJsonReader &jsonData)
    {
        std::string account;
        std::string password;
        if(!jsonData.TryGetValue("account", account)
           || !jsonData.TryGetValue("password", password))
        {
            return XCode::Failure;
        }
        auto iter = this->mOperAccountMap.find(account);
        if(iter == this->mOperAccountMap.end())
        {
            LOG_ERROR(account << " does not exist");
            return XCode::AccountNotExists;
        }
        if(iter->second != password)
        {
            LOG_ERROR(account << " password error");
            return XCode::Failure;
        }
        return XCode::Successful;
    }
}