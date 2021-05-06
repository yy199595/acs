#include"MysqlManager.h"
#include<CommonCore/Applocation.h>
#include<CommonUtil/NumberHelper.h>
#include<CommonThread/ThreadPool.h>
#include<MysqlClient/MysqlTaskAction.h>
#include<CommonCoroutine/CoroutineManager.h>
namespace SoEasy
{
	MysqlManager::MysqlManager()
	{
		
	}

	SayNoMysqlSocket * MysqlManager::GetMysqlSocket(long long threadId)
	{
		auto iter = this->mMysqlSocketMap.find(threadId);
		return iter != this->mMysqlSocketMap.end() ? iter->second : nullptr;
	}

	XMysqlErrorCode MysqlManager::QueryData(const char * db, const std::string & sql)
	{
		long long id = this->GetScheduler()->GetCurrentCorId();
		if (id == 0)
		{
			return XMysqlErrorCode::MysqlNotInCoroutine;
		}
		MysqlSharedTask taskAction = std::make_shared<MysqlTaskAction>(this, id, db, sql);

		if (ThreadPool::StartTaskAction(taskAction) == false)
		{
			return XMysqlErrorCode::MysqlStartTaskFail;
		}
		this->mTaskActionMap.insert(std::make_pair(taskAction->GetActionId(), taskAction));

		this->GetScheduler()->YieldReturn();
		return taskAction->GetQueryData()->GetErrorCode();
	}

	XMysqlErrorCode MysqlManager::QueryData(const char * db, const std::string & sql, std::shared_ptr<MysqlQueryData> & queryData)
	{
		long long id = this->GetScheduler()->GetCurrentCorId();
		if (id == 0)
		{
			return XMysqlErrorCode::MysqlNotInCoroutine;
		}
		MysqlSharedTask taskAction = std::make_shared<MysqlTaskAction>(this, id, db, sql);
		
		if (ThreadPool::StartTaskAction(taskAction) == false)
		{
			return XMysqlErrorCode::MysqlStartTaskFail;
		}
		this->mTaskActionMap.insert(std::make_pair(taskAction->GetActionId(), taskAction));

		this->GetScheduler()->YieldReturn();
		queryData = taskAction->GetQueryData();
		return queryData->GetErrorCode();
	}

	bool MysqlManager::OnInit()
	{
		ServerConfig & config = this->GetApp()->GetServerConfig();
		if (!config.GetValue(config.GetServerName(), "MysqlIp", this->mMysqlConfig.mIp))
		{
			SayNoDebugError("not find field 'MysqlIp'");
			return false;
		}

		if (!config.GetValue(config.GetServerName(), "MysqlPort", this->mMysqlConfig.mPort))
		{
			SayNoDebugError("not find field 'MysqlPort'");
			return false;
		}

		if (!config.GetValue(config.GetServerName(), "MysqlUserName", this->mMysqlConfig.mUserName))
		{
			SayNoDebugError("not find field 'MysqlUserName'");
			return false;
		}

		if (!config.GetValue(config.GetServerName(), "MysqlPassWord", this->mMysqlConfig.mPassCode))
		{
			SayNoDebugError("not find field 'MysqlPassWord'");
			return false;
		}
		return true;
	}

	void MysqlManager::OnTaskFinish(long long id)
	{
		auto iter = this->mTaskActionMap.find(id);
		if (iter != this->mTaskActionMap.end())
		{		
			MysqlSharedTask taskAction = iter->second;
			this->GetScheduler()->Resume(taskAction->GetActionId());
			this->mTaskActionMap.erase(iter);
		}
	}

	void MysqlManager::OnRegisterSuccessful(shared_ptr<TcpClientSession> tcpSession)
	{
		if (tcpSession->GetSessionName() == "CenterServer")
		{
			this->StartConnectMysql();
		}
	}

	bool MysqlManager::StartConnectMysql()
	{
		const char * ip = this->mMysqlConfig.mIp.c_str();
		const unsigned short port = this->mMysqlConfig.mPort;
		const char * passWd = this->mMysqlConfig.mPassCode.c_str();
		const char * userName = this->mMysqlConfig.mUserName.c_str();
		SayNoDebugLog("start connect mysql " << this->mMysqlConfig.ToString());

		std::vector<TaskThread *> taskThreads;
		ThreadPool::GetAllTaskThread(taskThreads);
		for (size_t index = 0; index < taskThreads.size(); index++)
		{
			TaskThread * taskThread = taskThreads[index];
			SayNoMysqlSocket * mysqlSocket1 = mysql_init((MYSQL*)0);
			SayNoMysqlSocket * mysqlSocket = mysql_real_connect(mysqlSocket1, ip, userName, passWd, NULL, port, NULL, CLIENT_MULTI_STATEMENTS);
			if (mysqlSocket == nullptr)
			{
				SayNoDebugError("connect mysql failure " << this->mMysqlConfig.ToString());
				return false;
			}
			long long id = taskThread->GetThreadId();
			this->mMysqlSocketMap.insert(std::make_pair(id, mysqlSocket));
		}

		SayNoDebugInfo("connect mysql successful " << this->mMysqlConfig.ToString());
		
			this->GetScheduler()->Start([this]()
			{
				while (true)
				{
					std::shared_ptr<MysqlQueryData> queryData;
					std::string sql = "select * from player_risk";
					const long long t1 = TimeHelper::GetMilTimestamp();
					XMysqlErrorCode code = this->QueryData("shouhuzhemen299_db", sql, queryData);
					if (code == XMysqlErrorCode::MysqlSuccessful)
					{
						const long long t2 = TimeHelper::GetMilTimestamp();
						SayNoDebugWarning("query data count " << queryData->GetColumnCount() << "  delay = " << t2 - t1);
					}
					this->GetScheduler()->Sleep(1000);
				}			
			});
		

		return true;
	}

	bool MysqlManager::ConnectRedis()
	{
		/*timeval timeOut;
		timeOut.tv_sec = 3000;
		timeOut.tv_usec = 3000;
		const char * ip = this->mRedisConfig.mIp.c_str();
		const unsigned short port = this->mRedisConfig.mPort;
		SayNoDebugLog("connect redis ip:" << ip << " port:" << port);
		this->mRedisReaderSession = redisConnectWithTimeout(ip, port, timeOut);
		this->mRedisWriterSession = redisConnectWithTimeout(ip, port, timeOut);*/
		return true;
	}
}