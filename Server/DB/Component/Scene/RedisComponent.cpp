#include"RedisComponent.h"
#include<Util/StringHelper.h>
#include<Scene/TaskComponent.h>
#include<Coroutine/CoroutineComponent.h>
#include<Script/ClassProxyHelper.h>
#include <Core/App.h>
namespace Sentry
{
    RedisComponent::RedisComponent()
    {
        this->mRedisPort = 0;
    }

	bool RedisComponent::CloseRedisSocket()
	{
		auto id = std::this_thread::get_id();
		auto iter = this->mRedisContextMap.find(id);
		if (iter != this->mRedisContextMap.end())
		{
			RedisSocket * socket = iter->second;
			redisFree(socket);
			this->mRedisContextMap.erase(iter);
			return true;
		}
		return false;
	}

	RedisSocket *RedisComponent::GetRedisSocket()
    {
		auto id = std::this_thread::get_id();
        auto iter = this->mRedisContextMap.find(id);
        return iter != this->mRedisContextMap.end() ? iter->second : nullptr;
    }

    bool RedisComponent::Awake()
    {
		ServerConfig & config = App::Get().GetConfig();
		SayNoAssertRetFalse_F(this->mTaskManager = Scene::GetComponent<TaskComponent>());
        SayNoAssertRetFalse_F(this->mCorComponent = Scene::GetComponent<CoroutineComponent>());
        

        SayNoAssertRetFalse_F(config.GetValue("Redis", "ip",this->mRedisIp));
        SayNoAssertRetFalse_F(config.GetValue("Redis", "port",this->mRedisPort));

        int second = 3;
		const std::vector<TaskThread*> & threads = this->mTaskManager->GetThreads();
		config.GetValue("Redis", "timeout", second);
		
		for (TaskThread * taskThread : threads)
		{
			redisContext * redisSocket = this->ConnectRedis(second);
			if(redisSocket == nullptr)
            {
                return false;
            }
			this->mRedisContextMap.emplace(taskThread->GetId(), redisSocket);
			SayNoDebugLog("connect redis successful [" << mRedisIp << ":" << mRedisPort << "]");
		}
        return true;
    }

	redisContext * RedisComponent::ConnectRedis(int timeout)
	{
		struct timeval tv;
		tv.tv_sec = 3;
		tv.tv_usec = tv.tv_sec * timeout * 1000;
		ServerConfig & config = App::Get().GetConfig();
		redisContext *pRedisContext = redisConnectWithTimeout(mRedisIp.c_str(), mRedisPort, tv);
		if (pRedisContext->err != 0)
		{
			SayNoDebugFatal(
				"connect redis fail " << mRedisIp << ":" << mRedisPort << " error:" << pRedisContext->errstr);
			return nullptr;
		}
		std::string redisPasswd;
		if (config.GetValue("Redis", "passwd", redisPasswd) && !redisPasswd.empty())
		{
			void * p = redisCommand(pRedisContext, "auth %s", redisPasswd.c_str());
			redisReply *reply = (redisReply *)redisCommand(pRedisContext, "auth %s", redisPasswd.c_str());
			if (reply == nullptr || reply->type == REDIS_REPLY_ERROR)
			{
				SayNoDebugError("redis Authentication failed " << reply->str);
				return nullptr;
			}
			freeReplyObject(reply);
		}
		return pRedisContext;
	}

    void RedisComponent::Start()
    {

    }

    bool RedisComponent::HasValue(const std::string &key)
    {
		RedisTask * redisTask = this->CreateTask("GET", key);
        if (this->mTaskManager->StartTask(redisTask) == 0)
        {
            return false;
        }
        this->mCorComponent->YieldReturn();
        return redisTask->GetErrorCode() == XCode::Successful;
    }

    bool RedisComponent::HasValue(const std::string &tab, const std::string &key)
    {
        RedisTask * redisTask = this->CreateTask("HEXISTS", tab, key);
        if (!this->mTaskManager->StartTask(redisTask))
        {
            return false;
        }
        this->mCorComponent->YieldReturn();
        return redisTask->GetErrorCode() == XCode::Successful;
    }

    bool RedisComponent::DelValue(const std::string &key)
    {
		RedisTask * redisTask = this->CreateTask("DEL", key);
        if (!this->mTaskManager->StartTask(redisTask))
        {
            return false;
        }
        this->mCorComponent->YieldReturn();
        return redisTask->GetErrorCode() == XCode::Successful;
    }

    bool RedisComponent::DelValue(const std::string &tab, const std::string &key)
    {
		RedisTask * redisTask = this->CreateTask("HDEL", tab, key);
        if (!this->mTaskManager->StartTask(redisTask))
        {
            return false;
        }
        this->mCorComponent->YieldReturn();
        return redisTask->GetErrorCode() == XCode::Successful;
    }

    bool RedisComponent::SetValue(const std::string &key, const std::string &value, int second)
    {
		RedisTask * redisTask = this->CreateTask("EXPIRE", key, second);
        if (!this->mTaskManager->StartTask(redisTask))
        {
            return false;
        }
        this->mCorComponent->YieldReturn();
        return redisTask->GetErrorCode() == XCode::Successful;
    }

    bool RedisComponent::SetValue(const std::string &key, const std::string &value)
    {
		RedisTask * redisTask = this->CreateTask("SET", key, value);
        if (!this->mTaskManager->StartTask(redisTask))
        {
            return false;
        }
        this->mCorComponent->YieldReturn();
        return redisTask->GetErrorCode() == XCode::Successful;
    }

    bool RedisComponent::SetValue(const std::string &tab, const std::string &key, const std::string &value)
    {
		RedisTask * redisTask = this->CreateTask("HSET", tab, key, value);
        if (!this->mTaskManager->StartTask(redisTask))
        {
            return false;
        }
        this->mCorComponent->YieldReturn();
        return redisTask->GetErrorCode() == XCode::Successful;

    }

    bool RedisComponent::SetValue(const std::string &tab, const std::string &key, const shared_ptr<Message> value)
    {
        std::string serializeData;
        if (!value->SerializePartialToString(&serializeData))
        {
            return false;
        }
        return this->SetValue(tab, key, serializeData);
    }

    bool RedisComponent::GetValue(const std::string &key, std::string &value)
    {
		RedisTask * redisTask = this->CreateTask("GET", key);
        if (!this->mTaskManager->StartTask(redisTask))
        {
            return false;
        }
        return redisTask->GetOnceData(value);
    }

    bool RedisComponent::GetValue(const std::string &tab, const std::string &key, std::string &value)
    {
		RedisTask * redisTask = this->CreateTask("HGET", tab, key);
        if (!this->mTaskManager->StartTask(redisTask))
        {
            return false;
        }
        return redisTask->GetOnceData(value);
    }

    bool RedisComponent::GetValue(const std::string &tab, const std::string &key, shared_ptr<Message> value)
    {
        std::string message;
        if (!this->GetValue(tab, key, message))
        {
            return false;
        }
        return value->ParseFromString(message);
    }
}
