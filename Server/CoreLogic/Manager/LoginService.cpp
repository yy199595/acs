#include "LoginService.h"
#include<Util/NumberHelper.h>
#include<NetWork/ActionScheduler.h>
namespace SoEasy
{
	bool LoginService::OnInit()
	{
		if (!this->GetConfig().GetValue("AreaId", this->mAreaId))
		{
			SayNoDebugFatal("not find field AreaId");
			return false;
		}
		REGISTER_FUNCTION_1(LoginService::Login, UserAccountData);
		REGISTER_FUNCTION_1(LoginService::Register, UserRegisterData);
		return true;
	}

	XCode LoginService::Login(long long operId, shared_ptr<UserAccountData> LoginData)
	{
		ActionScheduler actionShceduler;
		UserAccountData userAccountData;
		shared_ptr<StringData> accountData = make_shared<StringData>();

		const std::string & account = LoginData->account();
		const std::string & passswd = LoginData->passwd();
			
		accountData->set_data(account);
		XCode code = actionShceduler.Call("UserDataManager", "QueryUserData", accountData, userAccountData);
		if (code != XCode::Successful)
		{
			return XCode::AccountNotExists;
		}
		userAccountData.PrintDebugString();
		return code;
	}

	XCode LoginService::Register(long long operId, shared_ptr<UserRegisterData> registerData)
	{
		ActionScheduler actionShceduler;
		shared_ptr<UserAccountData> accountData = make_shared<UserAccountData>();

		const long long nowTime = TimeHelper::GetSecTimeStamp();
		const long long userId = NumberHelper::Create(this->mAreaId);

		accountData->set_user_id(userId);
		accountData->set_register_time(nowTime);
		accountData->set_account(registerData->account());
		accountData->set_passwd(registerData->password());
		accountData->set_phonenum(registerData->phonenum());
		accountData->set_platform(registerData->platform());
		accountData->set_device_mac(registerData->device_mac());
		return actionShceduler.Call("UserDataManager", "AddUserData", accountData);
	}
}