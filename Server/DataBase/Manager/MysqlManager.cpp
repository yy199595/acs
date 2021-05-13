#include"MysqlManager.h"
#include<Core/Applocation.h>
#include<Util/NumberHelper.h>
#include<Thread/ThreadPool.h>
#include<Coroutine/CoroutineManager.h>
#include<Coroutine/CoroutineManager.h>
#include<MysqlClient/MysqlTaskAction.h>
#include<Util/StringHelper.h>
namespace SoEasy
{
	MysqlManager::MysqlManager()
	{
		this->mThreadPool = 0;
		this->mMysqlPort = 0;
	}

	bool MysqlManager::OnInit()
	{
		this->mThreadPool = this->GetApp()->GetThreadPool();
		SayNoAssertRetFalse_F(this->mThreadPool);
		this->mCoroutineManager = this->GetManager<CoroutineManager>();
		SayNoAssertRetFalse_F(this->mCoroutineManager);

		std::string mysqlAddress;
		if (!GetConfig().GetValue("MysqlAddress", mysqlAddress))
		{
			SayNoDebugError("not find field 'MysqlAddress'");
			return false;
		}

		if (!StringHelper::ParseIpAddress(mysqlAddress, mMysqlIp, mMysqlPort))
		{
			SayNoDebugError("parse 'MysqlAddress' fail");
			return false;
		}

		if (!GetConfig().GetValue("MysqlUserName", mDataBaseUser))
		{
			SayNoDebugError("not find field 'MysqlUserName'");
			return false;
		}

		if (!GetConfig().GetValue("MysqlPassWord", mDataBasePasswd))
		{
			SayNoDebugError("not find field 'MysqlPassWord'");
			return false;
		}
		return this->StartConnectMysql();
	}

	SayNoMysqlSocket * MysqlManager::GetMysqlSocket(long long threadId)
	{
		auto iter = this->mMysqlSocketMap.find(threadId);
		return iter != this->mMysqlSocketMap.end() ? iter->second : nullptr;
	}

	shared_ptr<InvokeResultData> MysqlManager::InvokeCommand(const std::string db, const std::string & sql)
	{
		long long coreouitneId = this->mCoroutineManager->GetCurrentCorId();
		if (coreouitneId == 0)
		{
			return make_shared<InvokeResultData>(XCode::MysqlNotInCoroutine);
		}
		const long long taskId = NumberHelper::Create();
		MysqlSharedTask taskAction = make_shared<MysqlTaskAction>(this, taskId, coreouitneId, db, sql);
		
		if (!mThreadPool->StartTaskAction(taskAction))
		{		
			return make_shared<InvokeResultData>(XCode::MysqlStartTaskFail);
		}
		this->mTaskActionMap.insert(std::make_pair(taskAction->GetTaskId(), taskAction));

		this->mCoroutineManager->YieldReturn();

		XCode code = taskAction->GetCode();
		const std::string & error = taskAction->GetErrorStr();
		const std::string & jsonData = taskAction->GetJsonData();
		if (code != XCode::Successful)
		{
			SayNoDebugError("[mysql error] " << error);
		}
		return make_shared<InvokeResultData>(code, error, jsonData);
	}

	

	void MysqlManager::OnTaskFinish(long long id)
	{
		auto iter = this->mTaskActionMap.find(id);
		if (iter != this->mTaskActionMap.end())
		{		
			MysqlSharedTask taskAction = iter->second;
			const long long cor = taskAction->GetCoroutienId();
			this->mCoroutineManager->Resume(cor);
			this->mTaskActionMap.erase(iter);
		}
	}

	void MysqlManager::OnInitComplete()
	{
		
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
			SayNoMysqlSocket * mysqlSocket = mysql_real_connect(mysqlSocket1, ip, userName, passWd, NULL, port, NULL, CLIENT_MULTI_STATEMENTS);
			if (mysqlSocket == nullptr)
			{
				SayNoDebugError("connect mysql failure " << ip << ":" << port << "  " << userName << " " << passWd);
				return false;
			}
			this->mMysqlSocketMap.insert(std::make_pair(threadId, mysqlSocket));
			SayNoDebugInfo("connect mysql successful " << ip << ":" << port << "  " << userName << " " << passWd);
		}
		return true;
	}
}