#pragma once

#include<Protocol/c2s.pb.h>
#include<Service/ProtoServiceComponent.h>
#define USER_ID_START 7788
namespace GameKeeper
{

    class RedisComponent;
    class MysqlProxyComponent;


    class AccountService : public ProtoServiceComponent
    {
    public:
        AccountService();
        ~AccountService()  final = default;

    protected:
        bool Awake() override;

        void Start() final;
    private:
        XCode Login(const c2s::AccountLogin_Request& request, c2s::AccountLogin_Response & response);

        XCode Register(const c2s::AccountRegister_Request & request, c2s::AccountRegister_Response & response);

    private:
        const std::string NewToken(const std::string & account);
    private:
        RedisComponent *mRedisComponent;
        MysqlProxyComponent *mMysqlComponent;
    };
}// namespace GameKeeper