#include "AccountService.h"
#include <Core/App.h>
#include <Util/MD5.h>
#include <Util/MathHelper.h>
#include <Scene/RedisComponent.h>
#include <Scene/MysqlProxyComponent.h>
namespace Sentry
{
    AccountService::AccountService()
    {
    }

    bool AccountService::Awake()
    {
        __add_method(AccountService::Register);
        __add_method(AccountService::LoginByToken);
        __add_method(AccountService::LoginByPasswd);

        this->mRedisComponent = this->GetComponent<RedisComponent>();
        this->mMysqlComponent = this->GetComponent<MysqlProxyComponent>();
		return true;
    }

    void AccountService::Start()
    {

    }

    void AccountService::OnLodaData()
    {
        db::UserAccountData userAccountData;
        userAccountData.set_userid(112233445);
        userAccountData.set_passwd("199595yjz");
        userAccountData.set_devicemac("ios_qq");
        userAccountData.set_account("646585122@qq.com");
        userAccountData.set_lastlogintime(TimeHelper::GetSecTimeStamp());


        XCode code = this->mMysqlComponent->Add(userAccountData);

        this->mRedisComponent->ClearAllData();
        this->mRedisComponent->AddToSet("user", userAccountData.account());
        this->mRedisComponent->DelFromSet("user", userAccountData.account());

        this->mRedisComponent->AddToSet("user", userAccountData.account());


        if(this->mRedisComponent->SetValue("user", userAccountData.account(), userAccountData))
        {
            db::UserAccountData queryData;
            this->mRedisComponent->GetValue("user", userAccountData.account(), queryData);
            SayNoDebugLogProtocBuf(queryData);
        }

		userAccountData.Clear();
		userAccountData.set_account("646585122@qq.com");

        db::UserAccountData response;
        XCode responseCode = this->mMysqlComponent->Query(userAccountData, response);

        if(responseCode == XCode::Successful)
        {
            SayNoDebugLogProtocBuf(response);
        }
    }

    XCode AccountService::LoginByToken(long long operId, const c2s::AccountLogin_Request& request, c2s::AccountLogin_Response & response)
    {
        if (request.account().empty() || request.token().empty())
        {
            return XCode::Failure;
        }
        db::UserAccountData userAccountData;
        userAccountData.set_account(request.account());
        if (!this->mMysqlComponent->Query(userAccountData, userAccountData))
        {
            return XCode::AccountNotExists;
        }
        if (userAccountData.lastlogintime() != 0)
        {
            if(request.logindev() != userAccountData.devicemac())
            {
                return XCode::Successful;
            }
            const long long nowTime = TimeHelper::GetSecTimeStamp();
            if (nowTime - userAccountData.lastlogintime() >= 3 * 24 * 60 * 60)
            {
                return XCode::Failure; // token过期
            }
        }
        if(userAccountData.token() != request.token())
        {
            return XCode::Failure;
        }
        userAccountData.set_devicemac(request.logindev());
        userAccountData.set_token(this->NewToken(request.account()));
        userAccountData.set_lastlogintime(TimeHelper::GetSecTimeStamp());
        response.set_token(userAccountData.token());
        return XCode::Successful;
    }

    XCode AccountService::LoginByPasswd(long long operId, const c2s::AccountLogin_Request& request, c2s::AccountLogin_Response & response)
    {
        if (request.account().empty() || request.passwd().empty())
        {
            return XCode::Failure;
        }

    }

    XCode AccountService::Register(long long operId, const c2s::AccountRegister_Request & request, c2s::AccountRegister_Response & response)
    {
        const std::string &account = request.account();
        if (!this->mRedisComponent->AddToSet("Account", account))
        {
            return XCode::AccountAlreadyExists;
        }

        db::UserAccountData userAccountData;
        userAccountData.set_account(account);
        userAccountData.set_passwd(request.passwd());
        userAccountData.set_devicemac(request.logindev());
        userAccountData.set_token(this->NewToken(account));
        userAccountData.set_userid(NumberHelper::Create());
        userAccountData.set_registertime(TimeHelper::GetSecTimeStamp());

        if (this->mMysqlComponent->Add(userAccountData) != XCode::Successful)
        {
            return XCode::AccountAlreadyExists;
        }
        response.set_token(userAccountData.token());
        return XCode::Successful;
    }

    const std::string AccountService::NewToken(const std::string & account)
    {
        char buffer[100] = {0};
        int number = MathHelper::Random<int>();
        long long now = TimeHelper::GetSecTimeStamp();
        size_t size = sprintf(buffer, "%s:%lld:%d", account.c_str(), now, number);
        MD5 md5(buffer, size);
        return md5.toString();
    }
}// namespace Sentry