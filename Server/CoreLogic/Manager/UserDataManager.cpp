#include"UserDataManager.h"
#include<Manager/MysqlManager.h>
namespace SoEasy
{
	bool UserDataManager::OnInit()
	{
		if (!this->GetConfig().GetValue("AreaId", this->mAreaId))
		{
			SayNoDebugFatal("not find field AreaId");
			return false;
		}
		this->mMysqlManager = this->GetManager<MysqlManager>();
		SayNoAssertRetFalse_F(this->mMysqlManager);
		REGISTER_FUNCTION_1(UserDataManager::AddUserData, UserAccountData);
		return true;
	}

	XCode UserDataManager::AddUserData(long long operId, shared_ptr<UserAccountData> userData)
	{		
		std::stringstream sqlbuffer;
		const std::string registerTime = TimeHelper::GetDateString(userData->register_time());
		sqlbuffer << "insert into tb_player_account(account,platform,userid,passwd,phonenum,registertime)values(";
		sqlbuffer << "'" << userData->account() << "','" << userData->platform() << "'," << userData->user_id();
		sqlbuffer << ",'" << userData->passwd() << "'," << userData->phonenum() << ",'" << registerTime <<"');";

		const std::string addsql = sqlbuffer.str();
		std::shared_ptr<MysqlQueryData> queryResult;
		const long long t1 = TimeHelper::GetMilTimestamp();
		XCode code = this->mMysqlManager->QueryData("yjz", addsql, queryResult);
		if (code != XCode::Successful)
		{
			SayNoDebugError(queryResult->GetErrorMessage());
		}
		const long long t2 = TimeHelper::GetMilTimestamp();
		SayNoDebugInfo("const time = " << queryResult->GetConsumeTime() << "  " << t2 - t1);
		
		return code;
	}
}
