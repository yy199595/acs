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
		REGISTER_FUNCTION_1(UserDataManager::AddUserData, UserAccountData);
		SayNoAssertRetFalse_F(this->mRedisManager = this->GetManager<RedisManager>());
		SayNoAssertRetFalse_F(this->mMysqlManager = this->GetManager<MysqlManager>());
		return true;
	}

	XCode UserDataManager::AddUserData(long long operId, shared_ptr<UserAccountData> userData)
	{		
		const std::string & account = userData->account();
		shared_ptr<InvokeResultData> redisQuery = this->mRedisManager->InvokeCommand("HEXISTS tb_player_account %s", account.c_str());
		
		if (redisQuery->GetCode() == XCode::Successful && redisQuery->GetInt64() == 1)
		{
			return XCode::AccountAlreadyExists;
		}

		std::string accountData = userData->SerializeAsString();
		this->mRedisManager->InvokeCommand("HSET tb_player_account %s %b", account.c_str(), accountData.c_str(), accountData.size());
		
		std::stringstream sqlbuffer;
		const std::string registerTime = TimeHelper::GetDateString(userData->register_time());
		sqlbuffer << "insert into tb_player_account(account,platform,userid,passwd,phonenum,registertime)values(";
		sqlbuffer << "'" << userData->account() << "','" << userData->platform() << "'," << userData->user_id();
		sqlbuffer << ",'" << userData->passwd() << "'," << userData->phonenum() << ",'" << registerTime <<"');";

		const std::string addsql = sqlbuffer.str();	
		shared_ptr<InvokeResultData> queryResult = this->mMysqlManager->InvokeCommand("yjz", addsql);
		return queryResult->GetCode();
	}
}
