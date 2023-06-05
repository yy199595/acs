//
// Created by zmhy0073 on 2022/10/13.
//

#ifndef APP_USERBEHAVIOR_H
#define APP_USERBEHAVIOR_H

#include"Message/s2s/s2s.pb.h"
#include"Rpc/Service/RpcService.h"
namespace Tendo
{
    class Login : public RpcService
    {
    public:
        Login() = default;
        ~Login() = default;
    private:
		int OnLogin(long long id, const s2s::login::request & request);
		int OnLogout(long long id, const s2s::logout::request & request);
	private:
		bool OnInit() final;
	private:
		class ActorComponent * mActorComponent;
    };
}


#endif //APP_USERBEHAVIOR_H
