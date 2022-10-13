//
// Created by zmhy0073 on 2022/10/8.
//

#include "InnerService.h"
#include"Component/InnerNetComponent.h"
#include"Component/InnerNetMessageComponent.h"
#ifdef __ENABLE_MYSQL__
#include"Component/MysqlHelperComponent.h"
#endif

#ifdef __ENABLE_MONGODB__
#include"Component/MongoHelperComponent.h"
#endif
#include"Component/HttpComponent.h"
#include"Component/RedisSubComponent.h"
#include"Component/RedisDataComponent.h"
#include"Component/GateHelperComponent.h"
#include"Component/RedisRegistryComponent.h"
namespace Sentry
{
    void InnerService::Awake()
    {
        this->mInnerNetComponent = nullptr;
#ifdef __ENABLE_MYSQL__
        this->GetApp()->AddComponent<MysqlHelperComponent>();
#endif

#ifdef __ENABLE_MONGODB__
        this->GetApp()->AddComponent<MongoHelperComponent>();
#endif
        this->GetApp()->AddComponent<HttpComponent>();
        this->GetApp()->AddComponent<InnerNetComponent>();
        this->GetApp()->AddComponent<RedisSubComponent>();
        this->GetApp()->AddComponent<RedisDataComponent>();
        this->GetApp()->AddComponent<GateHelperComponent>();
        this->GetApp()->AddComponent<RedisRegistryComponent>();
        this->GetApp()->AddComponent<InnerNetMessageComponent>();
    }

    bool InnerService::OnStart()
    {
        BIND_COMMON_RPC_METHOD(InnerService::Ping);
        BIND_COMMON_RPC_METHOD(InnerService::Push);
        BIND_COMMON_RPC_METHOD(InnerService::Hotfix);
        BIND_COMMON_RPC_METHOD(InnerService::Login);
        BIND_COMMON_RPC_METHOD(InnerService::Logout);
        BIND_COMMON_RPC_METHOD(InnerService::StartService);
        BIND_COMMON_RPC_METHOD(InnerService::CloseService);
        this->mInnerNetComponent = this->GetComponent<InnerNetComponent>();
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

    XCode InnerService::Push(const s2s::location::push &request)
    {
        const std::string & service = request.name();
        Service * localService = this->GetApp()->GetService(service);
        if(localService == nullptr)
        {
            CONSOLE_LOG_ERROR("not find service : " << service);
            return XCode::Failure;
        }
        const std::string & address = request.address();
        localService->AddLocation(address, request.user_id());
        return XCode::Successful;
    }

    XCode InnerService::Login(const Rpc::Head & head, const s2s::location::sync & request)
    {
        std::string address;
        if(!head.Get("address", address))
        {
            return XCode::CallArgsError;
        }
        const InnerServerNode * serverInfo = this->mInnerNetComponent->GetSeverInfo(address);
        if(serverInfo == nullptr)
        {
            return XCode::Failure;
        }

        const std::string & service = request.name();
        IServiceUnitSystem * unitSystem = this->GetComponent<IServiceUnitSystem>(service);
        if(unitSystem != nullptr)
        {
            unitSystem->OnLogin(request.user_id());
        }
        CONSOLE_LOG_INFO(request.user_id() << " join service " << service);
        return XCode::Successful;
    }

    XCode InnerService::Logout(const Rpc::Head & head, const s2s::location::sync &request)
    {
        std::string client, address;
        return XCode::Successful;
    }

    XCode InnerService::Hotfix()
    {
        std::vector<Component *> components;
        this->GetApp()->GetComponents(components);
        for(Component * component : components)
        {
            IHotfix * hotfix = component->Cast<IHotfix>();
            if(hotfix != nullptr)
            {
                hotfix->OnHotFix();
            }
        }
        return XCode::Successful;
    }

    XCode InnerService::StartService(const com::type_string &request)
    {
        const std::string &name = request.str();
        Service *service = this->GetComponent<Service>(name);
        if (service == nullptr)
        {
            throw std::logic_error(fmt::format("not find service : {0}", name));
        }
        if (service->IsStartService())
        {
            throw std::logic_error(fmt::format("{0} already started", name));
        }
        if(!service->Start())
        {
            return XCode::Failure;
        }
        std::vector<IServiceChange *> components;
        this->GetApp()->GetComponents(components);
        for(IServiceChange * component : components)
        {
            component->OnAddService(name);
        }
        return XCode::Successful;
    }

    XCode InnerService::CloseService(const com::type_string &request)
    {
        const std::string &name = request.str();
        Service *service = this->GetComponent<Service>(name);
        if (service == nullptr)
        {
            throw std::logic_error(fmt::format("not find service : {0}", name));
        }
        if (!service->IsStartService())
        {
            throw std::logic_error(fmt::format("{0} not started", name));
        }
        if (!service->Close())
        {
            return XCode::Failure;
        }
        std::vector<IServiceChange *> components;
        this->GetApp()->GetComponents(components);
        for (IServiceChange *component: components)
        {
            component->OnDelService(name);
        }
        return XCode::Successful;
    }
}