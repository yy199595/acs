#include "LoginService.h"
#include<Util/NumberHelper.h>
#include<Manager/MysqlManager.h>
#include<Manager/RedisManager.h>
namespace SoEasy
{
	LoginService::LoginService()
	{

	}

	bool LoginService::OnInit()
	{
		REGISTER_FUNCTION_1(LoginService::Login, UserAccountData);
		REGISTER_FUNCTION_1(LoginService::Register, UserRegisterData);
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("AreaId", this->mAreaId));
		SayNoAssertRetFalse_F(this->mRedisManager = this->GetManager<RedisManager>());
		SayNoAssertRetFalse_F(this->mMysqlManager = this->GetManager<MysqlManager>());

		return LocalService::OnInit();
	}

	void LoginService::OnInitComplete()
	{
		shared_ptr<UserRegisterData> registerData = make_shared<UserRegisterData>();
		registerData->set_account("199595@qq.com");
		registerData->set_password("thispassword");
		registerData->set_phonenum(13716061995);
		XCode code = this->Register(0, registerData);

		if (code == XCode::Successful)
		{
			shared_ptr<UserAccountData> LoginData = make_shared<UserAccountData>();
			LoginData->set_account(registerData->account());
			if (this->Login(0, LoginData) == XCode::Successful)
			{
				SayNoDebugLog("Login Successful");
				return;
			}
			SayNoDebugFatal("Login Failure");
		}
	}

	XCode LoginService::Login(long long operId, shared_ptr<UserAccountData> LoginData)
	{
		const std::string table = "tb_player_account";
		const std::string & account = LoginData->account();
		shared_ptr<UserAccountData> userData = make_shared<UserAccountData>();
		if (!this->mRedisManager->GetValue(table, account, userData))
		{
			userData->set_account(LoginData->account());
			if (!this->mMysqlManager->QueryData(table, userData, "account"))
			{
				return XCode::AccountNotExists;
			}
			if (!this->mRedisManager->SetValue(table, account, userData))
			{
				SayNoDebugError("save user data to redis error " << account);
			}
		}
		SayNoDebugLogProtocBuf(*userData);
		return XCode::Successful;
	}

	XCode LoginService::Register(long long operId, shared_ptr<UserRegisterData> registerData)
	{
		const std::string table = "tb_player_account";
		shared_ptr<UserAccountData> userData = make_shared<UserAccountData>();
		userData->set_account(registerData->account());
		userData->set_passwd(registerData->password());
		userData->set_phonenum(registerData->phonenum());
		//userData->set_devicemac(registerData->device_mac());
		userData->set_platform(registerData->platform());
		userData->set_userid(NumberHelper::Create());
		userData->set_registertime(TimeHelper::GetSecTimeStamp());
		const std::string & account = registerData->account();
		if (this->mRedisManager->HasValue(table, account))
		{
			return XCode::AccountAlreadyExists;
		}
		if (!this->mMysqlManager->InsertData(table, userData))
		{
			if (this->mMysqlManager->QueryData(table, userData, "account"))
			{
				this->mRedisManager->SetValue("tb_player_account", account, userData);
				return XCode::AccountAlreadyExists;
			}
			SayNoDebugError("user account data error " << account);
			return XCode::Failure;
		}
		this->mRedisManager->SetValue(table, account, userData);
		return XCode::Successful;
	}
}