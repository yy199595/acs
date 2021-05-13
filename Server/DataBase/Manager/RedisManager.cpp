#include"RedisManager.h"
#include<Util/NumberHelper.h>
#include<Util/StringHelper.h>
#include<Thread/ThreadPool.h>
#include<NetWork/RemoteScheduler.h>
#include<RedisClient/RedisTaskAction.h>
#include<Coroutine/CoroutineManager.h>
namespace SoEasy
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
		std::string redisAddress;
		SayNoAssertRetFalse_F(this->mThreadPool = this->GetApp()->GetThreadPool());
		SayNoAssertRetFalse_F(this->mCoroutineScheduler = this->GetManager<CoroutineManager>());
		
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("RedisAddress", redisAddress));	
		SayNoAssertRetFalse_F(StringHelper::ParseIpAddress(redisAddress, mRedisIp, mRedisPort));
		
		struct timeval tv;
		tv.tv_sec = 3;
		tv.tv_usec = tv.tv_sec * 1000;
		if (this->GetConfig().GetValue("RedisTimeout", tv.tv_sec))
		{
			tv.tv_usec = tv.tv_sec * 1000;
		}

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
			std::string redisPasswd;
			if (this->GetConfig().GetValue("RedisPasswd", redisPasswd) && !redisPasswd.empty())
			{
				redisReply * reply = (redisReply *)redisCommand(pRedisContext, "AUTH %s", redisPasswd.c_str());
				if (reply == nullptr || reply->type == REDIS_REPLY_ERROR)
				{
					SayNoDebugError("redis Authentication failed " << reply->str);
					return false;
				}
				freeReplyObject(reply);
			}
		
			this->mRedisContextMap.emplace(threadId, pRedisContext);
			SayNoDebugLog("connect redis successful " << mRedisIp << ":" << mRedisPort);
		}
		
		return true;
	}

	void RedisManager::OnInitComplete()
	{
		//this->InvokeCommand("FLUSHALL");
		rapidjson::Value jsonData;
		shared_ptr<InvokeResultData> queryData = this->InvokeCommand("HGETALL tb_player_account");
		if (queryData->GetJsonData(jsonData) && jsonData.IsArray())
		{
			for (size_t index = 0; index < jsonData.Size(); index++)
			{
				const char * str = jsonData[index].GetString();
				const int num = jsonData[index].GetStringLength();
				SayNoDebugLog(std::string(str, num));
			}
		}
	}

	shared_ptr<InvokeResultData> RedisManager::InvokeCommand(const char * format, ...)
	{
		long long coroutineId = this->mCoroutineScheduler->GetCurrentCorId();
		if (coroutineId == 0)
		{
			return make_shared<InvokeResultData>(XCode::RedisNotInCoroutine);
		}
		va_list command;
		va_start(command, format);
		long long taskId = NumberHelper::Create();
		shared_ptr<RedisTaskAction> taskAction = make_shared<RedisTaskAction>(this, taskId, coroutineId);
		taskAction->InitCommand(format, command);
		va_end(command);
		if (!mThreadPool->StartTaskAction(taskAction))
		{
			return make_shared<InvokeResultData>(XCode::RedisStartTaskFail);
		}
		this->mTaskActionMap.insert(std::make_pair(taskAction->GetTaskId(), taskAction));
		this->mCoroutineScheduler->YieldReturn();
		

		XCode code = taskAction->GetCode();
		const std::string & error = taskAction->GetErrorStr();
		const std::string & jsonData = taskAction->GetJsonData();
		if (code != XCode::Successful)
		{
			SayNoDebugError("[redis error] " << error);
		}
		return make_shared<InvokeResultData>(code, error, jsonData);
	}
	void RedisManager::OnTaskFinish(long long id)
	{
		auto iter = this->mTaskActionMap.find(id);
		if (iter != this->mTaskActionMap.end())
		{
			shared_ptr<RedisTaskAction> taskAction = iter->second;
			long long coroutineId = taskAction->GetCoroutineId();
			this->mCoroutineScheduler->Resume(coroutineId);
			this->mTaskActionMap.erase(iter);
		}
	}
}
