#include "LoginService.h"
#include <Core/App.h>
#include <Scene/MysqlComponent.h>
#include <Scene/RedisComponent.h>
#include <Scene/MysqlProxyComponent.h>
#include <Scene/RedisComponent.h>
namespace Sentry
{
    LoginService::LoginService()
    {
    }

    bool LoginService::Awake()
    {
        //__add_method(LoginService::Login);
        //__add_method(LoginService::Register);
		return true;
    }

    void LoginService::Start()
    {

    }

    void LoginService::OnLodaData()
    {
        RedisComponent * redisComponent = this->GetComponent<RedisComponent>();
        MysqlProxyComponent * mysqlProxyComponent = this->GetComponent<MysqlProxyComponent>();

        db::UserAccountData userAccountData;
        userAccountData.set_userid(112233445);
        userAccountData.set_passwd("199595yjz");
        userAccountData.set_devicemac("ios_qq");
        userAccountData.set_account("646585122@qq.com");
        userAccountData.set_lastlogintime(TimeHelper::GetSecTimeStamp());
        XCode code = mysqlProxyComponent->Add(userAccountData);

        if(redisComponent->SetValue("user", userAccountData.account(), userAccountData))
        {
            db::UserAccountData queryData;
            redisComponent->GetValue("user", userAccountData.account(), queryData);
            SayNoDebugLogProtocBuf(queryData);
        }

		userAccountData.Clear();
		userAccountData.set_account("646585122@qq.com");

        db::UserAccountData response;
        XCode responseCode = mysqlProxyComponent->Query(userAccountData, response);

        if(responseCode == XCode::Successful)
        {
            SayNoDebugLogProtocBuf(response);
        }
    }

    XCode LoginService::Login(long long operId, const c2s::UserVerify_Request & LoginData)
    {
        return XCode::Failure;
    }

    XCode LoginService::Register(long long operId, const c2s::UserRegister_Request & registerData)
    {
        return XCode::Successful;
    }
}// namespace Sentry