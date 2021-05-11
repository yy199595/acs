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
		XCode Register(long long operId, shared_ptr<PB::PlayerRegisterData> registerData, shared_ptr<PB::PlayerRegisterBack> backData);
	};
}