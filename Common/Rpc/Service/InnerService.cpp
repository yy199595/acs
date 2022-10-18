//
// Created by zmhy0073 on 2022/10/8.
//

#include "InnerService.h"
#include"Component/InnerNetComponent.h"
#include"Component/RedisSubComponent.h"
#include"Component/RedisDataComponent.h"
#include"Component/RedisRegistryComponent.h"
#include"Component/InnerNetMessageComponent.h"
#include"Component/TextConfigComponent.h"
namespace Sentry
{
    bool InnerService::Awake()
    {
        this->mApp->AddComponent<InnerNetComponent>();
        this->mApp->AddComponent<RedisSubComponent>();
        this->mApp->AddComponent<RedisDataComponent>();
        this->mApp->AddComponent<RedisRegistryComponent>();
        this->mApp->AddComponent<InnerNetMessageComponent>();
        return true;
    }

    bool InnerService::OnStart()
    {
        BIND_COMMON_RPC_METHOD(InnerService::Ping);
        BIND_COMMON_RPC_METHOD(InnerService::Stop);
        BIND_COMMON_RPC_METHOD(InnerService::Hotfix);
        BIND_COMMON_RPC_METHOD(InnerService::LoadConfig);
        RedisSubComponent *subComponent = this->GetComponent<RedisSubComponent>();
        RedisDataComponent *dataComponent = this->GetComponent<RedisDataComponent>();
        InnerNetComponent *listenComponent = this->GetComponent<InnerNetComponent>();

        LOG_CHECK_RET_FALSE(subComponent && subComponent->StartConnectRedis());
        LOG_CHECK_RET_FALSE(dataComponent && dataComponent->StartConnectRedis());
        LOG_CHECK_RET_FALSE(listenComponent && listenComponent->StartListen("rpc"));
        return true;
    }

    XCode InnerService::Ping()
    {
        return XCode::Successful;
    }

    XCode InnerService::Stop()
    {
        std::vector<RpcService *> components;
        if(this->mApp->GetServices(components))
        {
            for (RpcService *component: components)
            {
                if(component != this && component->IsStartService())
                {
                    component->WaitAllMessageComplete();
                    component->Close();
                }
            }
        }
        this->mApp->Stop();
        return XCode::Successful;
    }

    XCode InnerService::LoadConfig()
    {
        TextConfigComponent * textComponent = this->GetComponent<TextConfigComponent>();
        if(textComponent != nullptr)
        {
            textComponent->OnHotFix();
        }
        return XCode::Successful;
    }

    XCode InnerService::Hotfix()
    {
        std::vector<IHotfix *> components;
        for(IHotfix * component : components)
        {
            component->OnHotFix();
        }
        return XCode::Successful;
    }
}