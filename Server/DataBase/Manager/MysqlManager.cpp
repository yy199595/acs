#include"MysqlManager.h"
#include<Core/Applocation.h>
#include<Util/NumberHelper.h>
#include<Thread/ThreadPool.h>
#include<Coroutine/CoroutineManager.h>
#include<MysqlClient/MysqlTaskAction.h>
#include<Util/StringHelper.h>
#include<Script/ClassProxyHelper.h>
#include<Script/MysqlExtension.h>

namespace SoEasy
{
	MysqlManager::MysqlManager()
	{
		this->mThreadPool = 0;
		this->mMysqlPort = 0;
	}

	bool MysqlManager::OnInit()
	{
		SayNoAssertRetFalse_F(this->mThreadPool = this->GetApp()->GetThreadPool());
		SayNoAssertRetFalse_F(this->mCoroutineManager = this->GetManager<CoroutineManager>());

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
		if (this->mCoroutineManager->IsInMainCoroutine())
		{
			return make_shared<InvokeResultData>(XCode::MysqlNotInCoroutine);
		}
		long long taskActionId = NumberHelper::Create();
		shared_ptr<MysqlTaskAction> taskAction = make_shared<MysqlTaskAction>(this, taskActionId, this->mCoroutineManager, db, sql);

		if (!this->StartTaskAction(taskAction))
		{
			return make_shared<InvokeResultData>(XCode::MysqlStartTaskFail);
		}
		this->mCoroutineManager->YieldReturn();

		return taskAction->GetInvokeData();
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