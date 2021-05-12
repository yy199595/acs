#pragma once
#include<Manager/Manager.h>
#include<Protocol/ServerCommon.pb.h>

namespace SoEasy
{
	class MysqlManager;
	class UserDataManager : public Manager
	{
	public:
		UserDataManager() { }
		~UserDataManager() { }
	protected:
		bool OnInit() override;
	public:

	private:
		XCode AddUserData(long long operId, shared_ptr<UserAccountData>);
	private:
		int mAreaId;
		MysqlManager * mMysqlManager;
	};
}