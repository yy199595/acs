#include"UserDataManager.h"
#include<Manager/MysqlManager.h>
#include<Manager/RedisManager.h>
namespace SoEasy
{
	bool UserDataManager::OnInit()
	{
		if (!this->GetConfig().GetValue("AreaId", this->mAreaId))
		{
			SayNoDebugFatal("not find field AreaId");
			return false;
		}
		this->mRedisManager = this->GetManager<RedisManager>();
		this->mMysqlManager = this->GetManager<MysqlManager>();
		SayNoAssertRetFalse_F(this->mRedisManager);
		SayNoAssertRetFalse_F(this->mMysqlManager);
		REGISTER_FUNCTION_1(UserDataManager::AddUserData, UserAccountData);
		return true;
	}

	XCode UserDataManager::AddUserData(long long operId, shared_ptr<UserAccountData> userData)
	{		
		const std::string & account = userData->account();
		shared_ptr<InvokeResultData> redisQuery = this->mRedisManager->InvokeCommand("HEXISTS tb_player_account %s", account.c_str());
		
		std::stringstream sqlbuffer;
		const std::string registerTime = TimeHelper::GetDateString(userData->register_time());
		sqlbuffer << "insert into tb_player_account(account,platform,userid,passwd,phonenum,registertime)values(";
		sqlbuffer << "'" << userData->account() << "','" << userData->platform() << "'," << userData->user_id();
		sqlbuffer << ",'" << userData->passwd() << "'," << userData->phonenum() << ",'" << registerTime <<"');";

		const std::string addsql = sqlbuffer.str();
		const long long t1 = TimeHelper::GetMilTimestamp();
		shared_ptr<InvokeResultData> queryResult = this->mMysqlManager->QueryData("yjz", addsql);
		if (queryResult->GetCode() != XCode::Successful)
		{
			SayNoDebugError(queryResult->GetErrorStr());
		}
		const long long t2 = TimeHelper::GetMilTimestamp();
		
		return queryResult->GetCode();
	}
}
