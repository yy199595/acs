#pragma once
#include<Service/LocalService.h>
#include<Protocol/c2s.pb.h>
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
		XCode Login(long long operId, shared_ptr<c2s::UserVerify_Request> LoginData);
		XCode Register(long long operId, shared_ptr<c2s::UserRegister_Request> registerData);
	private:
		int mAreaId;
		RedisManager * mRedisManager;
		MysqlManager * mMysqlManager;
	};
}