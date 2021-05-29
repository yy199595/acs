#pragma once
#include<Service/LocalService.h>
#include<Protocol/ServerCommon.pb.h>
namespace SoEasy
{
	class MysqlManager;
	class RedisManager;
	class LoginService : public LocalService
	{
	public:
		LoginService();
		~LoginService() { }
	protected:
		bool OnInit() override;
		void OnInitComplete() final;
	private:
		XCode Login(long long operId, shared_ptr<UserAccountData> LoginData);
		XCode Register(long long operId, shared_ptr<UserRegisterData> registerData);
	private:
		int mAreaId;
		RedisManager * mRedisManager;
		MysqlManager * mMysqlManager;
	};
}