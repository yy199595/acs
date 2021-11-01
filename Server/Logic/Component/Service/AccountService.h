#pragma once

#include <Protocol/c2s.pb.h>
#include <Service/LocalServiceComponent.h>

namespace GameKeeper
{

    class RedisComponent;
    class MysqlProxyComponent;


    class AccountService : public LocalServiceComponent, public ILoadData
    {
    public:
        AccountService();

        ~AccountService() {}

    protected:
        bool Awake() override;

        void Start() final;

        void OnLodaData() final;

    private:
        XCode LoginByToken(long long operId, const c2s::AccountLogin_Request& request, c2s::AccountLogin_Response & response);

        XCode LoginByPasswd(long long operId, const c2s::AccountLogin_Request& request, c2s::AccountLogin_Response & response);

        XCode Register(long long operId, const c2s::AccountRegister_Request & request, c2s::AccountRegister_Response & response);

    private:
        const std::string NewToken(const std::string & account);
    private:
        RedisComponent *mRedisComponent;
        MysqlProxyComponent *mMysqlComponent;
    };
}// namespace GameKeeper