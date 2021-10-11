#pragma once

#include <Protocol/c2s.pb.h>
#include <Service/LocalServiceComponent.h>

namespace Sentry
{
    class MysqlComponent;

    class RedisComponent;

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
        XCode Login(long long operId, const c2s::AccountLogin_Request& request, c2s::AccountLogin_Response & response);

        XCode Register(long long operId, const c2s::AccountRegister_Request & request, c2s::AccountRegister_Response & response);

    private:
        RedisComponent *mRedisManager;
        MysqlComponent *mMysqlManager;
    };
}// namespace Sentry