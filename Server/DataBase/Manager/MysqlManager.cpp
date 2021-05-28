#include"MysqlManager.h"
#include<Core/Applocation.h>
#include<Util/NumberHelper.h>
#include<Thread/ThreadPool.h>
#include<Coroutine/CoroutineManager.h>
#include<MysqlClient/MysqlTaskAction.h>
#include<Util/StringHelper.h>
#include<Script/ClassProxyHelper.h>
#include<Script/MysqlExtension.h>
#include<Util/FileHelper.h>
namespace SoEasy
{
	MysqlManager::MysqlManager()
	{
		this->mThreadPool = 0;
		this->mMysqlPort = 0;
	}

	bool MysqlManager::OnInit()
	{
		std::string mysqlAddress;
		SayNoAssertRetFalse_F(this->mThreadPool = this->GetApp()->GetThreadPool());
		SayNoAssertRetFalse_F(this->mCoroutineManager = this->GetManager<CoroutineManager>());

		SayNoAssertRetFalse_F(GetConfig().GetValue("SqlTable", mSqlTablePath));
		SayNoAssertRetFalse_F(GetConfig().GetValue("MysqlAddress", mysqlAddress));
		SayNoAssertRetFalse_F(GetConfig().GetValue("MysqlDbName", mDataBaseName));
		SayNoAssertRetFalse_F(GetConfig().GetValue("MysqlUserName", mDataBaseUser));
		SayNoAssertRetFalse_F(GetConfig().GetValue("MysqlPassWord", mDataBasePasswd));
		SayNoAssertRetFalse_F(StringHelper::ParseIpAddress(mysqlAddress, mMysqlIp, mMysqlPort));
		return this->StartConnectMysql();
	}

	SayNoMysqlSocket * MysqlManager::GetMysqlSocket(long long threadId)
	{
		auto iter = this->mMysqlSocketMap.find(threadId);
		return iter != this->mMysqlSocketMap.end() ? iter->second : nullptr;
	}

	shared_ptr<InvokeResultData> MysqlManager::InvokeCommand(const std::string db, const std::string & sql)
	{
		if (this->mCoroutineManager->IsInMainCoroutine())
		{
			return make_shared<InvokeResultData>(XCode::MysqlNotInCoroutine);
		}
		long long taskActionId = NumberHelper::Create();
		shared_ptr<MysqlTaskAction> taskAction = make_shared<MysqlTaskAction>(this, taskActionId, db, sql, this->mCoroutineManager);

		if (!this->StartTaskAction(taskAction))
		{
			return make_shared<InvokeResultData>(XCode::MysqlStartTaskFail);
		}
		this->mCoroutineManager->YieldReturn();

		return taskAction->GetInvokeData();
	}

	bool MysqlManager::CreateMysqlDb()
	{
		if (mysql_select_db(this->mMysqlSockt, this->mDataBaseName.c_str()) != 0)
		{
			std::string sql = "Create DataBase " + this->mDataBaseName;
			if (mysql_real_query(mMysqlSockt, sql.c_str(), sql.length()) != 0)
			{
				SayNoDebugError("create " << this->mDataBaseName << " db fail");
				return false;
			}
			SayNoDebugLog("create " << this->mDataBaseName << " db successful");
		}
		return this->CreateMysqlTable();
	}

	bool MysqlManager::CreateMysqlTable()
	{
		std::string jsonConfig;
		if (!FileHelper::ReadTxtFile(this->mSqlTablePath, jsonConfig))
		{
			SayNoDebugFatal("not find sql config " << jsonConfig);
			return false;
		}
		rapidjson::Document jsonDocument;
		jsonDocument.Parse(jsonConfig.c_str(), jsonConfig.size());
		if (jsonDocument.HasParseError())
		{
			SayNoDebugFatal("parse json fail " << jsonConfig);
			return false;
		}
		for (auto iter = jsonDocument.MemberBegin(); iter != jsonDocument.MemberEnd(); iter++)
		{
			const char * tableName = iter->name.GetString();
			if (!iter->value.IsObject() || !this->CreateMysqlTable(tableName, &iter->value))
			{
				SayNoDebugError("create sql table fail " << tableName);
				return false;
			}
		}
		return true;
	}

	bool MysqlManager::CreateMysqlTable(const char * tab, rapidjson::Value * jsonValue)
	{
		std::string sql = "desc " + std::string(tab);
		if (mysql_real_query(mMysqlSockt, sql.c_str(), sql.length()) != 0)
		{
			return this->CreateNewMysqlTable(tab, jsonValue);
		}
		return true;
	}
	bool MysqlManager::CreateNewMysqlTable(const char * tab, rapidjson::Value * jsonValue)
	{
		std::stringstream sqlCommand;
		auto iter1 = jsonValue->FindMember("primaty_key");
		if (iter1 == jsonValue->MemberEnd())
		{
			return false;	//没有设置主键
		}
		std::string paimaty = iter1->value.GetString();

		auto iter2 = jsonValue->FindMember(paimaty.c_str());
		if (iter2 == jsonValue->MemberEnd())
		{
			return false;		//字段没有主键
		}

		sqlCommand << "create table `" << tab << "`(\n";
		for (auto iter = jsonValue->MemberBegin(); iter != jsonValue->MemberEnd(); iter++)
		{
			std::string key = iter->name.GetString();
			if (key != "primaty_key")
			{
				if (!iter->value.IsArray())
				{
					return false;
				}
				rapidjson::Value jsonArray = iter->value.GetArray();
				sqlCommand << "`" << key << "` " << jsonArray[0].GetString();
				sqlCommand << " comment '" << jsonArray[1].GetString() << "',\n";
			}
		}
		sqlCommand << "PRIMARY KEY (`" << paimaty << "`)\n";
		sqlCommand << ")ENGINE=InnoDB DEFAULT CHARSET = utf8;";
		const std::string sql = sqlCommand.str();
		if (mysql_real_query(mMysqlSockt, sql.c_str(), sql.length()) != 0)
		{
			SayNoDebugError(mysql_error(mMysqlSockt));
			return false;
		}	
		return true;
	}

	void MysqlManager::OnInitComplete()
	{
		
	}

	void MysqlManager::PushClassToLua(lua_State * luaEnv)
	{
		ClassProxyHelper::PushStaticExtensionFunction(luaEnv, "SoEasy", "InvokeMysqlCommand", SoEasy::InvokeMysqlCommand);
	}

	bool MysqlManager::StartConnectMysql()
	{
		const char * ip = this->mMysqlIp.c_str();
		const unsigned short port = this->mMysqlPort;
		const char * passWd = this->mDataBasePasswd.c_str();
		const char * userName = this->mDataBaseUser.c_str();
		
		std::vector<long long> taskThreads;
		this->mThreadPool->GetAllTaskThread(taskThreads);
		for (size_t index = 0; index < taskThreads.size(); index++)
		{
			long long threadId = taskThreads[index];
			SayNoMysqlSocket * mysqlSocket1 = mysql_init((MYSQL*)0);
			this->mMysqlSockt = mysql_real_connect(mysqlSocket1, ip, userName, passWd, NULL, port, NULL, CLIENT_MULTI_STATEMENTS);
			if (this->mMysqlSockt == nullptr)
			{
				SayNoDebugError("connect mysql failure " << ip << ":" << port << "  " << userName << " " << passWd);
				return false;
			}
			this->mMysqlSocketMap.insert(std::make_pair(threadId, this->mMysqlSockt));
			SayNoDebugInfo("connect mysql successful " << ip << ":" << port << "  " << userName << " " << passWd);
		}
		return this->CreateMysqlDb();
	}
}