//
// Created by zmhy0073 on 2022/10/13.
//

#ifndef APP_USERBEHAVIOR_H
#define APP_USERBEHAVIOR_H

#include"Message/s2s/s2s.pb.h"
#include"Rpc/Service/PhysicalRpcService.h"
namespace Tendo
{
    class User : public PhysicalRpcService
    {
    public:
        User() = default;
        ~User() = default;
    private:
        int Login(const s2s::user::login & request);
        int Logout(const s2s::user::logout & request);
    private:
		bool OnInit() final;
        bool Awake() final;
        bool OnStart() final;
    private:
		class NodeMgrComponent * mNodeComponent;
        class InnerNetComponent * mInnerNetComponent;
    };
}


#endif //APP_USERBEHAVIOR_H
