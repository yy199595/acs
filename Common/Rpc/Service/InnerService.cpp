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
#include"Component/LocationComponent.h"
#include"Config/ClusterConfig.h"
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
        BIND_COMMON_RPC_METHOD(InnerService::Join);
        BIND_COMMON_RPC_METHOD(InnerService::Exit);
        BIND_COMMON_RPC_METHOD(InnerService::Stop);
        BIND_COMMON_RPC_METHOD(InnerService::Hotfix);
        BIND_COMMON_RPC_METHOD(InnerService::LoadConfig);
        this->mLocationComponent = this->GetComponent<LocationComponent>();
        LOG_CHECK_RET_FALSE(this->GetComponent<RedisSubComponent>()->StartConnectRedis());
        LOG_CHECK_RET_FALSE(this->GetComponent<RedisDataComponent>()->StartConnectRedis());
        return true;
    }

    XCode InnerService::Ping()
    {
        return XCode::Successful;
    }

    XCode InnerService::Join(const s2s::cluster::join &request)
    {
        const NodeConfig *nodeConfig = ClusterConfig::Inst()->GetConfig(request.name());
        if (nodeConfig == nullptr)
        {
            LOG_ERROR("not find cluster config : " << request.name());
            return XCode::Failure;
        }
        std::vector<std::string> services;
        if (nodeConfig->GetServices(services) <= 0)
        {
            return XCode::Failure;
        }
        for (const std::string &service: services)
        {
            if (RpcConfig::Inst()->GetConfig(service) != nullptr)
            {
                this->mLocationComponent->AddLocation(service, request.rpc());
            }
            else if (HttpConfig::Inst()->GetConfig(service) != nullptr)
            {
                this->mLocationComponent->AddLocation(service, request.http());
            }
        }
        return XCode::Successful;
    }

    XCode InnerService::Exit(const s2s::cluster::exit &response)
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

        TimerComponent * timerComponent = this->mApp->GetTimerComponent();
        timerComponent->DelayCall(10 * 1000, [this]()
        {
            this->mApp->OnDestory();
            this->mApp->Stop();
        });
        CONSOLE_LOG_INFO("服务器将在" << 10 << "秒后关闭");
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