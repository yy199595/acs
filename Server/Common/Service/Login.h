//
// Created by zmhy0073 on 2022/10/13.
//

#ifndef APP_USERBEHAVIOR_H
#define APP_USERBEHAVIOR_H

#include"Message/s2s/s2s.pb.h"
#include"Rpc/Service/RpcService.h"
namespace joke
{
    class Login : public RpcService
    {
    public:
        Login();
        ~Login() override = default;
    private:
		int OnLogin(long long id, const s2s::login::request & request);
		int OnLogout(long long id, const s2s::logout::request & request);
	private:
		bool OnInit() final;
	private:
		std::vector<ILogin *> mLoginComponents;
		class ActorComponent * mActorComponent;
		class MasterComponent * mMasterComponent;
	};
}


#endif //APP_USERBEHAVIOR_H
