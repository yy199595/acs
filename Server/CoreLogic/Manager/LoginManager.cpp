#include "LoginManager.h"
#include<Util/NumberHelper.h>
#include<NetWork/ActionScheduler.h>
namespace SoEasy
{
	bool LoginManager::OnInit()
	{
		if (!this->GetConfig().GetValue("AreaId", this->mAreaId))
		{
			SayNoDebugFatal("not find field AreaId");
			return false;
		}
		REGISTER_FUNCTION_1(LoginManager::Register, UserRegisterData);
		return true;
	}

	XCode LoginManager::Register(long long operId, shared_ptr<UserRegisterData> registerData)
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
		return actionShceduler.Call("UserDataManager.AddUserData", accountData);
	}
}