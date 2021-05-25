#include "LoginService.h"
#include<Util/NumberHelper.h>
#include<NetWork/ActionScheduler.h>
namespace SoEasy
{
	LoginService::LoginService()
	{

	}

	bool LoginService::OnInit()
	{
		SayNoAssertRetFalse_F(LocalService::OnInit());
		REGISTER_FUNCTION_1(LoginService::Login, UserAccountData);
		REGISTER_FUNCTION_1(LoginService::Register, UserRegisterData);
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("AreaId", this->mAreaId));
		return true;
	}

	XCode LoginService::Login(long long operId, shared_ptr<UserAccountData> LoginData)
	{
		
		return XCode::Successful;
	}

	XCode LoginService::Register(long long operId, shared_ptr<UserRegisterData> registerData)
	{
		return XCode::Successful;
	}
}