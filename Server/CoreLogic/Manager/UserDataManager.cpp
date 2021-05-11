#include"UserDataManager.h"
#include<Util/MD5.h>
#include<Util/NumberHelper.h>
#include<Util/StringHelper.h>
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
		REGISTER_FUNCTION_2(UserDataManager::AddUserData, PlayerRegisterData, PlayerRegisterData);
		return true;
	}

	XCode UserDataManager::AddUserData(long long operId, shared_ptr<PlayerRegisterData> userData, shared_ptr<PlayerRegisterData> backData)
	{		
		char sql[1024] = { 0 };
		const int areaId = userData->area_id();
		const std::string & acount = userData->account();
		const std::string & passwd = userData->password();
		
		const std::string & token = "01021x25s121d1sad1as3";
		const long long nowTime = TimeHelper::GetSecTimeStamp();
		const long long userId = NumberHelper::Create(this->mAreaId);
		size_t size = sprintf_s(sql, "insert into tb_player_account(account,areaud,userid,passwd,registertime,lastlogintime,lastloginip,token)values('%s',%d,%lld,'%s',%lld,%lld,'%s','%s')",
			acount.c_str(), areaId, userId, passwd.c_str(), nowTime, 0, "127.0.0.1", token.c_str());
		XCode code = this->mMysqlManager->QueryData("yjz", std::string(sql, size));
		if (code == XCode::Successful)
		{
			
		}
		return code;
	}
}
