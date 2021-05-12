#pragma once
#include<Manager/Manager.h>
#include<Protocol/ServerCommon.pb.h>
namespace SoEasy
{
	class LoginManager : public Manager
	{
	public:
		LoginManager() { }
		~LoginManager() { }
	protected:
		bool OnInit() override;
	private:
		XCode Register(long long operId, shared_ptr<UserRegisterData> registerData);
	private:
		int mAreaId;
	};
}