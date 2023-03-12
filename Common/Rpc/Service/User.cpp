//
// Created by zmhy0073 on 2022/10/13.
//

#include"User.h"
#include"Component/InnerNetComponent.h"
#include"Component/NodeMgrComponent.h"
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
		this->mLocationComponent = this->GetComponent<NodeMgrComponent>();
        this->mInnerNetComponent = this->GetComponent<InnerNetComponent>();
        return true;
    }

    int User::Login(const s2s::user::login & request)
    {
        return XCode::Successful;
    }

    int User::Logout(const s2s::user::logout &request)
    {
        return XCode::Successful;
    }

}