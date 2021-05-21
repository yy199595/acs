#include"UserDataManager.h"
#include<Manager/MysqlManager.h>
#include<Manager/RedisManager.h>
namespace SoEasy
{
	bool UserDataManager::OnInit()
	{
		SayNoAssertRetFalse_F(ServiceBase::OnInit());
		REGISTER_FUNCTION_1(UserDataManager::AddUserData, UserAccountData);
		REGISTER_FUNCTION_2(UserDataManager::QueryUserData, StringData, UserAccountData);

		SayNoAssertRetFalse_F(this->GetConfig().GetValue("AreaId", this->mAreaId))
		SayNoAssertRetFalse_F(this->mRedisManager = this->GetManager<RedisManager>());
		SayNoAssertRetFalse_F(this->mMysqlManager = this->GetManager<MysqlManager>());
		return true;
	}

	XCode UserDataManager::AddUserData(long long operId, shared_ptr<UserAccountData> userData)
	{		
		const std::string & account = userData->account();
		if (this->mRedisManager->HasValue("tb_player_account", account.c_str()))
		{
			return XCode::AccountAlreadyExists;
		}
		if (!this->mRedisManager->SetValue("tb_player_account", account.c_str(), userData))
		{
			return XCode::SaveValueToRedisFail;
		}

		std::stringstream sqlbuffer;
		const std::string registerTime = TimeHelper::GetDateString(userData->register_time());
		sqlbuffer << "insert into tb_player_account(account,platform,userid,passwd,phonenum,registertime)values(";
		sqlbuffer << "'" << userData->account() << "','" << userData->platform() << "'," << userData->user_id();
		sqlbuffer << ",'" << userData->passwd() << "'," << userData->phonenum() << ",'" << registerTime <<"');";

		shared_ptr<InvokeResultData> queryResult = this->mMysqlManager->InvokeCommand("yjz", sqlbuffer.str());
		return queryResult->GetCode();
	}

	XCode UserDataManager::QueryUserData(long long operId, shared_ptr<StringData> account, shared_ptr<UserAccountData> queryData)
	{
		const std::string & userAccount = account->data();
		if (this->mRedisManager->GetValue("tb_player_account", userAccount.c_str(), queryData))
		{
			return XCode::Successful;
		}
		std::stringstream sqlbuffer;
		sqlbuffer << "select from tb_player_account where account=" << userAccount << ";";
		shared_ptr<InvokeResultData> queryResult = this->mMysqlManager->InvokeCommand("yjz", sqlbuffer.str());
		if (queryResult->GetCode() != XCode::Successful)
		{
			return queryResult->GetCode();
		}
		rapidjson::Value jsonValue;
		if (!queryResult->GetJsonData(jsonValue) || jsonValue.IsObject())
		{
			return XCode::Failure;
		}
		queryData->set_user_id(jsonValue["userid"].GetInt64());
		queryData->set_passwd(jsonValue["passwd"].GetString());
		queryData->set_account(jsonValue["account"].GetString());
		queryData->set_platform(jsonValue["platform"].GetString());
		queryData->set_phonenum(jsonValue["phonenum"].GetInt64());
		queryData->set_register_time(jsonValue["registertime"].GetInt64());

		this->mRedisManager->SetValue("tb_player_account", userAccount.c_str(), queryData);
		return XCode::Successful;
	}
}
