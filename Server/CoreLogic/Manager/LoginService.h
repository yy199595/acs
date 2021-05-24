#pragma once
#include<Service/LocalService.h>
#include<Protocol/ServerCommon.pb.h>
namespace SoEasy
{
	class LoginService : public LocalService
	{
	public:
		LoginService();
		~LoginService() { }
	protected:
		bool OnInit() override;
	private:
		XCode Login(long long operId, shared_ptr<UserAccountData> LoginData);
		XCode Register(long long operId, shared_ptr<UserRegisterData> registerData);
	private:
		int mAreaId;
	};
}