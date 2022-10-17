//
// Created by zmhy0073 on 2022/10/13.
//

#include"UserBehavior.h"
#include"Component/InnerNetComponent.h"
#include"Component/LocationComponent.h"
#include"Component/GateHelperComponent.h"
namespace Sentry
{

    bool UserBehavior::Awake()
    {
        this->mApp->AddComponent<GateHelperComponent>();
        return true;
    }
    bool UserBehavior::OnStart()
    {
        BIND_COMMON_RPC_METHOD(UserBehavior::Push);
        BIND_COMMON_RPC_METHOD(UserBehavior::Login);
        BIND_COMMON_RPC_METHOD(UserBehavior::Logout);
		this->mLocationComponent = this->GetComponent<LocationComponent>();
        this->mInnerNetComponent = this->GetComponent<InnerNetComponent>();
        return true;
    }

    XCode UserBehavior::Push(const s2s::location::push &request)
    {
        const std::string & service = request.name();
        RpcService * localService = this->mApp->GetService(service);
        if(localService == nullptr)
        {
            CONSOLE_LOG_ERROR("not find service : " << service);
            return XCode::Failure;
        }
        const std::string & address = request.address();
		this->mLocationComponent->AddLocation(service, address);
        return XCode::Successful;
    }

    XCode UserBehavior::Login(const Rpc::Head & head, const s2s::location::sync & request)
    {
        std::string address;
        if(!head.Get("address", address))
        {
            return XCode::CallArgsError;
        }
        const InnerClienData * serverInfo = this->mInnerNetComponent->GetSeverInfo(address);
        if(serverInfo == nullptr)
        {
            return XCode::Failure;
        }
		LocationUnit * locationUnit = this->mLocationComponent->AddLocationUnit(request.user_id());
		if(locationUnit != nullptr)
		{
			
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

    XCode UserBehavior::Logout(const Rpc::Head & head, const s2s::location::sync &request)
    {
        std::string client, address;
        return XCode::Successful;
    }

}