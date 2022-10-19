//
// Created by zmhy0073 on 2022/10/13.
//

#ifndef APP_USERBEHAVIOR_H
#define APP_USERBEHAVIOR_H
#include"LocalRpcService.h"
namespace Sentry
{
    class UserBehavior : public LocalRpcService
    {
    public:
        UserBehavior() = default;
        ~UserBehavior() = default;
    private:
        XCode Login(const Rpc::Head & head, const s2s::user::login & request);
        XCode Logout(const Rpc::Head & head, const s2s::user::logout & request);
    private:
        bool Awake() final;
        bool OnStart() final;
        bool OnClose() final { return false; }
    private:
		class LocationComponent * mLocationComponent;
        class InnerNetComponent * mInnerNetComponent;
    };
}


#endif //APP_USERBEHAVIOR_H
