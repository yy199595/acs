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
        long long phoneNumber = 0;
        string user_account, user_password;
        LOG_THROW_ERROR(request.TryGetValue("account", user_account));
        LOG_THROW_ERROR(request.TryGetValue("password", user_password));
        LOG_THROW_ERROR(request.TryGetValue("phone_num", phoneNumber));
        long long userId = this->mRedisComponent->AddCounter("UserId");

        db::db_account::tab_user_account userAccountData;

        userAccountData.set_user_id(userId);
        userAccountData.set_account(user_account);
        userAccountData.set_phone_num(phoneNumber);
        userAccountData.set_password(user_password);
        userAccountData.set_register_time(Helper::Time::GetSecTimeStamp());
        return this->mMysqlComponent->Add(userAccountData)->GetCode();
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