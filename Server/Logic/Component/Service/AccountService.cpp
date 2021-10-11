#include "AccountService.h"
#include <Core/App.h>
#include <Scene/MysqlComponent.h>
#include <Scene/RedisComponent.h>
#include <Scene/MysqlProxyComponent.h>
#include <Scene/RedisComponent.h>
namespace Sentry
{
    AccountService::AccountService()
    {
    }

    bool AccountService::Awake()
    {
        //__add_method(AccountService::Login);
        //__add_method(AccountService::Register);
		return true;
    }

    void AccountService::Start()
    {

    }

    void AccountService::OnLodaData()
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

        redisComponent->ClearAllData();
        redisComponent->AddToSet("user", userAccountData.account());
        redisComponent->DelFromSet("user", userAccountData.account());

        redisComponent->AddToSet("user", userAccountData.account());


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

    XCode AccountService::Login(long long operId, const c2s::AccountLogin_Request& request, c2s::AccountLogin_Response & response)
    {
        return XCode::Successful;
    }

    XCode AccountService::Register(long long operId, const c2s::AccountRegister_Request & request, c2s::AccountRegister_Response & response)
    {
        return XCode::Successful;
    }
}// namespace Sentry