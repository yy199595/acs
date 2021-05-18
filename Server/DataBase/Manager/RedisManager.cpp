#include"RedisManager.h"
#include<Util/NumberHelper.h>
#include<Util/StringHelper.h>
#include<Thread/ThreadPool.h>
#include<NetWork/RemoteScheduler.h>
#include<RedisClient/RedisTaskAction.h>
#include<Coroutine/CoroutineManager.h>
#include<Script/ClassProxyHelper.h>
#include<Script/MysqlExtension.h>
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
		std::string value;
		this->SetValue("name", std::string("19959510"));
		this->GetValue("name", value);
		SayNoDebugInfo("name = " << value);
		this->mCoroutineScheduler->Sleep(6000);
		value = "";
		this->GetValue("name", value);
		SayNoDebugFatal("name = " << value);
	}

	void RedisManager::PushClassToLua(lua_State * luaEnv)
	{
		ClassProxyHelper::PushStaticExtensionFunction(luaEnv, "SoEasy", "InvokeRedisCommand", SoEasy::InvokeRedisCommand);
	}

	bool RedisManager::HasValue(const char * key)
	{
		shared_ptr<InvokeResultData> queryData = this->InvokeCommand("GET", key);
		if (queryData->GetCode() == XCode::Successful)
		{
			rapidjson::Value jsonValue;
			return queryData->GetJsonData(jsonValue) && !jsonValue.IsNull();
		}
		return false;
	}

	bool RedisManager::HasValue(const char * tab, const char * key)
	{
		shared_ptr<InvokeResultData> queryData = this->InvokeCommand("HEXISTS", tab, key);
		if (queryData->GetCode() == XCode::Successful)
		{
			return queryData->GetInt64() == 1;
		}
		return false;
	}

	bool RedisManager::DelValue(const char * key)
	{
		shared_ptr<InvokeResultData> queryData = this->InvokeCommand("DEL", key);
		return queryData->GetCode() == XCode::Successful;
	}

	bool RedisManager::DelValue(const char * tab, const char * key)
	{
		shared_ptr<InvokeResultData> queryData = this->InvokeCommand("HDEL", tab, key);
		return queryData->GetCode() == XCode::Successful;
	}

	bool RedisManager::SetValue(const char * key, const std::string & value, int second)
	{
		if (!this->HasValue(key))
		{
			this->SetValue(key, value);
		}
		shared_ptr<InvokeResultData> queryData = this->InvokeCommand("EXPIRE", key, second);
		return queryData->GetCode() == XCode::Successful;
	}

	bool RedisManager::SetValue(const char * key, const std::string & value)
	{
		shared_ptr<InvokeResultData> queryData = this->InvokeCommand("SET", key, value);
		return queryData->GetCode() == XCode::Successful;
	}

	bool RedisManager::SetValue(const char * tab, const char * key, const std::string & value)
	{
		shared_ptr<InvokeResultData> queryData = this->InvokeCommand("HSET", tab, key, value);
		return queryData->GetCode() == XCode::Successful;
		
	}

	bool RedisManager::SetValue(const char * tab, const char * key, const shared_ptr<Message> value)
	{
		std::string serializeData;
		if (!value->SerializePartialToString(&serializeData))
		{
			return false;
		}
		return this->SetValue(tab, key, serializeData);
	}

	bool RedisManager::GetValue(const char * key, std::string & value)
	{
		shared_ptr<InvokeResultData> queryData = this->InvokeCommand("GET", key);
		if (queryData->GetCode() == XCode::Successful)
		{
			rapidjson::Value jsonValue;
			if (queryData->GetJsonData(jsonValue) && jsonValue.IsString())
			{
				const char * str = jsonValue.GetString();
				const size_t size = jsonValue.GetStringLength();
				value.assign(str, size);
				return true;
			}
		}
		return false;
	}

	bool RedisManager::GetValue(const char * tab, const char * key, std::string & value)
	{
		shared_ptr<InvokeResultData> queryData = this->InvokeCommand("HGET", tab, key);
		if (queryData->GetCode() == XCode::Successful)
		{
			rapidjson::Value jsonValue;
			if (queryData->GetJsonData(jsonValue) && jsonValue.IsString())
			{
				const char * str = jsonValue.GetString();
				const size_t size = jsonValue.GetStringLength();
				value.assign(str, size);
				return true;
			}
		}
		return false;
	}

	bool RedisManager::GetValue(const char * tab, const char * key, shared_ptr<Message> value)
	{
		std::string message;
		if (!this->GetValue(tab, key, message))
		{
			return false;
		}
		return value->ParseFromString(message);
	}
}
