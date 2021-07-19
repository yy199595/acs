#include "LoginService.h"
#include<Util/NumberHelper.h>
#include<Manager/MysqlManager.h>
#include<Manager/RedisManager.h>
namespace Sentry
{
	LoginService::LoginService()
	{

	}

	bool LoginService::OnInit()
	{
		

		return LocalService::OnInit();
	}

	void LoginService::OnInitComplete()
	{
		
	}

	XCode LoginService::Login(long long operId, shared_ptr<c2s::UserVerify_Request> LoginData)
	{
		return XCode::Failure;
	}

	XCode LoginService::Register(long long operId, shared_ptr<c2s::UserRegister_Request> registerData)
	{
		return XCode::Successful;
	}
}