#pragma once

#include"Protocol/c2s.pb.h"
#include"Component/Service/HttpService.h"
#define USER_ID_START 7788
namespace Sentry
{

    class ServiceProxy;
    class RedisComponent;
    class MysqlProxyComponent;
    class AccountService : public HttpService
    {
    public:
        AccountService() = default;
        ~AccountService()  final = default;

    protected:
        bool Awake() final;
        bool LateAwake() final;

    private:
        XCode Login(const RapidJsonReader & request, RapidJsonWriter & response);

        XCode Register(const RapidJsonReader & request, RapidJsonWriter & response);

    private:
        const std::string NewToken(const std::string & account);
    private:
        RedisComponent *mRedisComponent;
        MysqlProxyComponent *mMysqlComponent;
        std::shared_ptr<ServiceProxy> mGateService;
    };
}// namespace Sentry