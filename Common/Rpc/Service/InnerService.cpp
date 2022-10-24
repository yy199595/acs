//
// Created by zmhy0073 on 2022/10/8.
//

#include"InnerService.h"
#include"Config/ClusterConfig.h"
#include"Component/InnerNetComponent.h"
#include"Component/LocationComponent.h"
#include"Component/TextConfigComponent.h"
#include"Component/InnerNetMessageComponent.h"

namespace Sentry
{
    bool InnerService::Awake()
    {
        this->mApp->AddComponent<InnerNetComponent>();
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
		SUB_EVENT_MESSAGE("Test1", InnerService::OnInvoke);
		SUB_EVENT_MESSAGE("Test2", InnerService::OnInvoke2);
		this->mLocationComponent = this->GetComponent<LocationComponent>();
        return true;
    }

	void InnerService::OnInvoke(const std::string& message)
	{

	}

	void InnerService::OnInvoke2(const std::string & message)
	{

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

		CONSOLE_LOG_INFO("shutdown server int 10s after");
		TimerComponent * timerComponent = this->mApp->GetTimerComponent();
		timerComponent->DelayCall(10 * 1000, &App::Stop, this->mApp);
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