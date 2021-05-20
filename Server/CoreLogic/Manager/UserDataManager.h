#pragma once
#include<Service/ServiceBase.h>
#include<Protocol/ServerCommon.pb.h>

namespace SoEasy
{
	class MysqlManager;
	class RedisManager;
	class UserDataManager : public ServiceBase
	{
	public:
		UserDataManager() { }
		~UserDataManager() { }
	protected:
		bool OnInit() override;
	private:
		XCode AddUserData(long long operId, shared_ptr<UserAccountData>);
		XCode QueryUserData(long long operId, shared_ptr<StringData> account, shared_ptr<UserAccountData>);
	private:
		int mAreaId;
		MysqlManager * mMysqlManager;
		RedisManager * mRedisManager;
	};
}