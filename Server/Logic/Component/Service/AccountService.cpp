#include "AccountService.h"
#include "Object/App.h"
#include <Util/MD5.h>
#include <Util/MathHelper.h>
#include "Component/RedisComponent.h"
#include "Component/MysqlProxyComponent.h"
#include"MysqlClient/MysqlRpcTaskSource.h"
namespace Sentry
{
    bool AccountService::Awake()
    {
        this->mRedisComponent = nullptr;
        this->mMysqlComponent = nullptr;
        BIND_HTTP_FUNCTION(AccountService::Login);
        BIND_HTTP_FUNCTION(AccountService::Register);
		return true;
    }

    bool AccountService::LateAwake()
    {
        this->mRedisComponent = this->GetComponent<RedisComponent>();
        this->mMysqlComponent = this->GetComponent<MysqlProxyComponent>();
        return true;
    }

    XCode AccountService::Login(const RapidJsonReader &request, RapidJsonWriter &response)
    {
        return XCode::Successful;
    }

    XCode AccountService::Register(const RapidJsonReader &request, RapidJsonWriter &response)
    {
        return XCode::Successful;
    }

    const std::string AccountService::NewToken(const std::string & account)
    {
        char buffer[100] = {0};
        int number = Helper::Math::Random<int>();
        long long now = Helper::Time::GetSecTimeStamp();
        size_t size = sprintf(buffer, "%s:%lld:%d", account.c_str(), now, number);
        return Helper::Md5::GetMd5(buffer, size);
    }
}// namespace Sentry