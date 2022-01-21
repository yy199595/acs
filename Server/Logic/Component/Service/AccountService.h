#pragma once

#include<Protocol/c2s.pb.h>
#include<Service/ServiceComponentBase.h>
#define USER_ID_START 7788
namespace Sentry
{

    class RedisComponent;
    class MysqlProxyComponent;
    class AccountService : public ServiceComponentBase
    {
    public:
        AccountService() = default;
        ~AccountService()  final = default;

    protected:
        bool Awake() final;
        bool LateAwake() final;


    private:
        XCode Login(const c2s::AccountLogin_Request& request, c2s::AccountLogin_Response & response);

        XCode Register(const c2s::AccountRegister_Request & request, c2s::AccountRegister_Response & response);

    private:
        const std::string NewToken(const std::string & account);
    private:
        RedisComponent *mRedisComponent;
        MysqlProxyComponent *mMysqlComponent;
    };
}// namespace Sentry