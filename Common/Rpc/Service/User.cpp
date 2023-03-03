//
// Created by zmhy0073 on 2022/10/13.
//

#include"User.h"
#include"Component/InnerNetComponent.h"
#include"Component/LocationComponent.h"
#include"Component/GateHelperComponent.h"
namespace Sentry
{

    bool User::Awake()
    {
        this->mApp->AddComponent<GateHelperComponent>();
        return true;
    }
    bool User::OnStart()
    {
        BIND_COMMON_RPC_METHOD(User::Login);
        BIND_COMMON_RPC_METHOD(User::Logout);
		this->mLocationComponent = this->GetComponent<LocationComponent>();
        this->mInnerNetComponent = this->GetComponent<InnerNetComponent>();
        return true;
    }

    XCode User::Login(const Rpc::Head & head, const s2s::user::login & request)
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

    XCode User::Logout(const Rpc::Head & head, const s2s::user::logout &request)
    {
        std::string client, address;
        return XCode::Successful;
    }

}