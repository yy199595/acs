#include "MysqlManager.h"
#include <fstream>
#include <Core/Applocation.h>
#include <Util/NumberHelper.h>
#include <Thread/ThreadPool.h>
#include <Coroutine/CoroutineManager.h>
#include <MysqlClient/MysqlTaskAction.h>
#include <Util/StringHelper.h>
#include <Script/ClassProxyHelper.h>
#include <Script/MysqlExtension.h>
#include <Util/FileHelper.h>

#include <MysqlClient/TableOperator.h>
#include <MysqlClient/MysqlQueryTask.h>
#include <MysqlClient/MysqlInsertTask.h>
#include <MysqlClient/MysqlUpdateTask.h>
#include <MysqlClient/MysqlDeleteTask.h>
#include <Protocol/db.pb.h>
namespace SoEasy
{
	SqlTableConfig::SqlTableConfig(const std::string tab, const std::string pb)
		: mTableName(tab), mProtobufName(pb)
	{
	}
	void SqlTableConfig::AddKey(const std::string key)
	{
		this->mKeys.push_back(key);
	}
	bool SqlTableConfig::HasKey(const std::string &key)
	{
		for (const std::string &k : this->mKeys)
		{
			if (k == key)
			{
				return true;
			}
		}
		return false;
	}
}

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

		SayNoAssertRetFalse_F(this->StartConnectMysql());
		SayNoAssertRetFalse_F(this->InitMysqlTable());
		return true;
	}

	SayNoMysqlSocket *MysqlManager::GetMysqlSocket(long long threadId)
	{
		auto iter = this->mMysqlSocketMap.find(threadId);
		return iter != this->mMysqlSocketMap.end() ? iter->second : nullptr;
	}

	bool MysqlManager::GetTableName(const std::string &pb, std::string &table)
	{
		auto iter = this->mTablePbMap.find(pb);
		if (iter != this->mTablePbMap.end())
		{
			table = iter->second;
			return true;
		}
		return false;
	}

	SqlTableConfig *MysqlManager::GetTableConfig(const std::string &tab)
	{
		auto iter = this->mSqlConfigMap.find(tab);
		return iter != this->mSqlConfigMap.end() ? iter->second : nullptr;
	}

	shared_ptr<InvokeResultData> MysqlManager::InvokeCommand(const std::string &sql)
	{
		if (this->mCoroutineManager->IsInMainCoroutine())
		{
			return make_shared<InvokeResultData>(XCode::MysqlNotInCoroutine);
		}
		long long taskActionId = NumberHelper::Create();
		shared_ptr<MysqlTaskAction> taskAction = make_shared<MysqlTaskAction>(this, taskActionId, this->mDataBaseName, sql, this->mCoroutineManager);

		if (!this->StartTaskAction(taskAction))
		{
			return make_shared<InvokeResultData>(XCode::MysqlStartTaskFail);
		}
		this->mCoroutineManager->YieldReturn();

		return taskAction->GetInvokeData();
	}

	XCode MysqlManager::StartTask(shared_ptr<MysqlTaskBase> task, Message * messageData)
	{
		if(this->mCoroutineManager->IsInMainCoroutine())
		{
			return XCode::NoCoroutineContext;
		}
		std::string table;
		if (!this->GetTableName(messageData->GetTypeName(), table))
		{
			SayNoDebugError(messageData->GetTypeName() << " not found sql table");
			return XCode::CallArgsError;
		}
		task->InitTask(table, this->mCoroutineManager, messageData);
		if(!this->StartTaskAction(task))
		{
			return XCode::MysqlStartTaskFail;
		}
		return XCode::Successful;
	}

	void MysqlManager::OnInitComplete()
	{
		
	}

	void MysqlManager::PushClassToLua(lua_State *luaEnv)
	{
		ClassProxyHelper::PushStaticExtensionFunction(luaEnv, "SoEasy", "InvokeMysqlCommand", SoEasy::InvokeMysqlCommand);
	}

	bool MysqlManager::InitMysqlTable()
	{
		std::fstream fs(this->mSqlTablePath, std::ios::in);
		if (!fs.is_open())
		{
			SayNoDebugError("not find file " << this->mSqlTablePath);
			return false;
		}
		std::string json;
		std::string temp;
		while (std::getline(fs, temp))
		{
			json.append(temp);
		}
		rapidjson::Document document;
		document.Parse(json.c_str(), json.size());
		if (document.HasParseError())
		{
			SayNoDebugError("parse " << mSqlTablePath << " json fail");
			return false;
		}
		for (auto iter = document.MemberBegin(); iter != document.MemberEnd(); iter++)
		{
			if (!iter->name.IsString() || !iter->value.IsObject())
			{
				SayNoDebugError(mSqlTablePath << " error");
				return false;
			}
			auto iter1 = iter->value.FindMember("keys");
			auto iter2 = iter->value.FindMember("protobuf");
			if (iter1 == iter->value.MemberEnd() || !iter1->value.IsArray())
			{
				SayNoDebugError(mSqlTablePath << " error");
			}
			if (iter2 == iter->value.MemberEnd() || !iter2->value.IsString())
			{
				SayNoDebugError(mSqlTablePath << " error");
			}
			const std::string tab = iter->name.GetString();
			const std::string pb = iter2->value.GetString();
			SqlTableConfig *tabConfig = new SqlTableConfig(tab, pb);
			for (unsigned int index = 0; index < iter1->value.Size(); index++)
			{
				tabConfig->AddKey(iter1->value[index].GetString());
			}
			this->mTablePbMap.emplace(pb, tab);
			this->mSqlConfigMap.emplace(tab, tabConfig);
		}
		TableOperator tableOper(this->mMysqlSockt, this->mDataBaseName, document);
		return tableOper.InitMysqlTable();
	}

	bool MysqlManager::StartConnectMysql()
	{
		const char *ip = this->mMysqlIp.c_str();
		const unsigned short port = this->mMysqlPort;
		const char *passWd = this->mDataBasePasswd.c_str();
		const char *userName = this->mDataBaseUser.c_str();

		std::vector<long long> taskThreads;
		this->mThreadPool->GetAllTaskThread(taskThreads);
		for (size_t index = 0; index < taskThreads.size(); index++)
		{
			long long threadId = taskThreads[index];
			SayNoMysqlSocket *mysqlSocket1 = mysql_init((MYSQL *)0);
			this->mMysqlSockt = mysql_real_connect(mysqlSocket1, ip, userName, passWd, NULL, port, NULL, CLIENT_MULTI_STATEMENTS);
			if (this->mMysqlSockt == nullptr)
			{
				SayNoDebugError("connect mysql failure " << ip << ":" << port << "  " << userName << " " << passWd);
				return false;
			}
			this->mMysqlSocketMap.insert(std::make_pair(threadId, this->mMysqlSockt));
			SayNoDebugInfo("connect mysql successful " << ip << ":" << port << "  " << userName << " " << passWd);
		}
		return true;
	}
}