#include"RedisManager.h"
#include<Util/StringHelper.h>
#include<Thread/ThreadPool.h>
#include<NetWork/RemoteScheduler.h>
#include<RedisClient/RedisTaskAction.h>
#include<Coroutine/CoroutineManager.h>
namespace DataBase
{
	RedisManager::RedisManager()
	{
		this->mRedisPort = 0;
		this->mThreadPool = nullptr;
	}

	RedisSocket * RedisManager::GetRedisSocket(long long id)
	{
		auto iter = this->mRedisContextMap.find(id);
		return iter != this->mRedisContextMap.end() ? iter->second : nullptr;
	}

	bool RedisManager::OnInit()
	{
		this->mThreadPool = this->GetApp()->GetThreadPool();
		this->mCoroutineScheduler = this->GetManager<CoroutineManager>();
		SayNoAssertRetFalse_F(this->mThreadPool);
		SayNoAssertRetFalse_F(this->mCoroutineScheduler);


		std::string redisAddress;
		if (!this->GetConfig().GetValue("RedisAddress", redisAddress))
		{
			SayNoDebugFatal("not find field RedisAddress");
			return false;
		}

		if (!StringHelper::ParseIpAddress(redisAddress, mRedisIp, mRedisPort))
		{
			SayNoDebugFatal("parse RedisAddress fail");
			return false;
		}
		
		struct timeval tv;
		tv.tv_sec = 2000 / 1000;
		tv.tv_usec = (2000 % 1000) * 1000;
		std::vector<long long> threadVector;
		this->mThreadPool->GetAllTaskThread(threadVector);

		for (size_t index = 0; index < threadVector.size(); index++)
		{
			long long threadId = threadVector[index];
			redisContext * pRedisContext = redisConnectWithTimeout(mRedisIp.c_str(), mRedisPort, tv);
			if (pRedisContext->err != 0)
			{
				SayNoDebugFatal("connect redis fail " << mRedisIp << ":" << mRedisPort << " error:" << pRedisContext->errstr);
				return false;
			}
			this->mRedisContextMap.emplace(threadId, pRedisContext);
			SayNoDebugLog("connect redis successful " << mRedisIp << ":" << mRedisPort);
		}
		return true;
	}
	void RedisManager::OnSecondUpdate()
	{
		
		
	}

	void RedisManager::OnFrameUpdate(float t)
	{
		RemoteScheduler shceuder;
		StringArray queryInfo;
		queryInfo.add_data_array()->assign("shouhuzhemen_account");
		queryInfo.add_data_array()->assign("spack_risk_season");
		long long t1 = TimeHelper::GetMilTimestamp();
		shceuder.Call("MysqlManager.QueryTable", &queryInfo, [t1](shared_ptr<TcpClientSession>, XCode code)
		{
			long long t2 = TimeHelper::GetMilTimestamp();
			SayNoDebugWarning("cost time = " << t2 - t1);
		});
	}
}
