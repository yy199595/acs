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
        BIND_COMMON_RPC_METHOD(UserBehavior::Login);
        BIND_COMMON_RPC_METHOD(UserBehavior::Logout);
		this->mLocationComponent = this->GetComponent<LocationComponent>();
        this->mInnerNetComponent = this->GetComponent<InnerNetComponent>();
        return true;
    }

    XCode UserBehavior::Login(const Rpc::Head & head, const s2s::user::login & request)
    {
        std::string address;
        if(!head.Get("address", address))
        {
            return XCode::CallArgsError;
        }
        const ServiceNodeInfo * serverInfo = this->mInnerNetComponent->GetSeverInfo(address);
        if(serverInfo == nullptr)
        {
            return XCode::Failure;
        }
        long long userId = request.user_id();
		std::unique_ptr<LocationUnit> locationUnit(new LocationUnit(userId, address));
		if(locationUnit != nullptr)
		{
			
		}
        return XCode::Successful;
    }

    XCode UserBehavior::Logout(const Rpc::Head & head, const s2s::user::logout &request)
    {
        std::string client, address;
        return XCode::Successful;
    }

}