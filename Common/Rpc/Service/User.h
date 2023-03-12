//
// Created by zmhy0073 on 2022/10/13.
//

#ifndef APP_USERBEHAVIOR_H
#define APP_USERBEHAVIOR_H
#include"PhysicalService.h"
#include"Message/s2s.pb.h"
namespace Sentry
{
    class User : public PhysicalService
    {
    public:
        User() = default;
        ~User() = default;
    private:
        int Login(const Rpc::Head & head, const s2s::user::login & request);
        int Logout(const Rpc::Head & head, const s2s::user::logout & request);
    private:
        bool Awake() final;
        bool OnStart() final;
    private:
		class NodeMgrComponent * mLocationComponent;
        class InnerNetComponent * mInnerNetComponent;
    };
}


#endif //APP_USERBEHAVIOR_H
