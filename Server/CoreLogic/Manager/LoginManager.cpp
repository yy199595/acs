#include "LoginManager.h"
#include<NetWork/ActionScheduler.h>
namespace SoEasy
{
	bool LoginManager::OnInit()
	{
		REGISTER_FUNCTION_2(LoginManager::Register, PlayerRegisterData, PlayerRegisterBack);
		return true;
	}

	XCode LoginManager::Register(long long operId, shared_ptr<PB::PlayerRegisterData> registerData, shared_ptr<PB::PlayerRegisterBack> backData)
	{
		ActionScheduler actionShceduler;
		return actionShceduler.Call("UserDataManager.AddUserData", registerData);
	}
}