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

#include<MysqlClient/TableOperator.h>
#include<MysqlClient/MysqlQueryTask.h>
#include<MysqlClient/MysqlInsertTask.h>
#include<MysqlClient/MysqlUpdateTask.h>
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

	shared_ptr<InvokeResultData> MysqlManager::InvokeCommand(const std::string & sql)
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

	bool MysqlManager::InsertData(const std::string tab, shared_ptr<Message> data)
	{
		SayNoAssertRetFalse_F(this->mCoroutineManager->IsInLogicCoroutine());
		long long taskActionId = NumberHelper::Create();
		shared_ptr<MysqlInsertTask> taskAction = make_shared<MysqlInsertTask>(this, taskActionId, this->mDataBaseName);

		SayNoAssertRetFalse_F(taskAction->InitTask(tab, this->mCoroutineManager, data));
		
		SayNoAssertRetFalse_F(this->StartTaskAction(taskAction));
		this->mCoroutineManager->YieldReturn();

		return taskAction->GetErrorCode() == XCode::Successful;
	}

	bool MysqlManager::QueryData(const std::string tab, shared_ptr<Message> data, const std::string & key)
	{
		SayNoAssertRetFalse_F(this->mCoroutineManager->IsInLogicCoroutine());
		
		long long taskActionId = NumberHelper::Create();
		shared_ptr<MysqlQueryTask> taskAction = make_shared<MysqlQueryTask>(this, taskActionId, this->mDataBaseName);
		SayNoAssertRetFalse_F(taskAction->InitTask(tab, this->mCoroutineManager, key, data));
		
		SayNoAssertRetFalse_F(this->StartTaskAction(taskAction));
		this->mCoroutineManager->YieldReturn();
		return taskAction->GetErrorCode() == XCode::Successful;
	}

	bool MysqlManager::UpdateData(const std::string tab, shared_ptr<Message> data, const std::string & key)
	{
		SayNoAssertRetFalse_F(this->mCoroutineManager->IsInLogicCoroutine());
		
		long long taskActionId = NumberHelper::Create();
		shared_ptr<MysqlUpdateTask> taskAction = make_shared<MysqlUpdateTask>(this, taskActionId, this->mDataBaseName);
		SayNoAssertRetFalse_F(taskAction->InitTask(tab, this->mCoroutineManager, key, data));
		
		SayNoAssertRetFalse_F(this->StartTaskAction(taskAction));
		
		this->mCoroutineManager->YieldReturn();
		return taskAction->GetErrorCode() == XCode::Successful;
	}

	bool MysqlManager::DeleteData(const std::string tab, shared_ptr<Message> data, const std::string & key)
	{
		return false;
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
		TableOperator tableOper(this->mMysqlSockt, this->mDataBaseName, this->mSqlTablePath);	
		return tableOper.InitMysqlTable();
	}
}