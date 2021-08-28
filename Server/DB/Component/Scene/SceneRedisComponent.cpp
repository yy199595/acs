#include"SceneRedisComponent.h"
#include<Util/StringHelper.h>
#include<Scene/SceneTaskComponent.h>
#include<Coroutine/CoroutineComponent.h>
#include<Script/ClassProxyHelper.h>
#include <Core/App.h>
namespace Sentry
{
    SceneRedisComponent::SceneRedisComponent()
    {
        this->mRedisPort = 0;
    }

	bool SceneRedisComponent::CloseRedisSocket()
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

	RedisSocket *SceneRedisComponent::GetRedisSocket()
    {
		auto id = std::this_thread::get_id();
        auto iter = this->mRedisContextMap.find(id);
        return iter != this->mRedisContextMap.end() ? iter->second : nullptr;
    }

    bool SceneRedisComponent::Awake()
    {
		ServerConfig & config = App::Get().GetConfig();
		SayNoAssertRetFalse_F(this->mTaskManager = Scene::GetComponent<SceneTaskComponent>());
        SayNoAssertRetFalse_F(this->mCorComponent = Scene::GetComponent<CoroutineComponent>());
        

        SayNoAssertRetFalse_F(config.GetValue("Redis", "ip",this->mRedisIp));
        SayNoAssertRetFalse_F(config.GetValue("Redis", "port",this->mRedisPort));

        int second = 3;
		std::vector<std::thread::id> threads;
		this->mTaskManager->GetThreads(threads);
		config.GetValue("Redis", "timeout", second);
		
		for (std::thread::id & id : threads)
		{
			redisContext * redisSocket = this->ConnectRedis(second);
			SayNoAssertRetFalse_F(redisSocket);
			this->mRedisContextMap.emplace(id, redisSocket);
			SayNoDebugLog("connect redis successful [" << mRedisIp << ":" << mRedisPort << "]  [ id = " << id << "]");
		}
        return true;
    }

	redisContext * SceneRedisComponent::ConnectRedis(int timeout)
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
		if (config.GetValue("RedisPasswd", redisPasswd) && !redisPasswd.empty())
		{
			redisReply *reply = (redisReply *)redisCommand(pRedisContext, "AUTH %s", redisPasswd.c_str());
			if (reply == nullptr || reply->type == REDIS_REPLY_ERROR)
			{
				SayNoDebugError("redis Authentication failed " << reply->str);
				return nullptr;
			}
			freeReplyObject(reply);
		}
		return pRedisContext;
	}

    void SceneRedisComponent::Start()
    {

    }

    bool SceneRedisComponent::HasValue(const std::string &key)
    {
		RedisTask * redisTask = this->CreateTask("GET", key);
        if (this->mTaskManager->StartTask(redisTask) == 0)
        {
            return false;
        }
        this->mCorComponent->YieldReturn();
        return redisTask->GetErrorCode() == XCode::Successful;
    }

    bool SceneRedisComponent::HasValue(const std::string &tab, const std::string &key)
    {
        RedisTask * redisTask = this->CreateTask("HEXISTS", tab, key);
        if (!this->mTaskManager->StartTask(redisTask))
        {
            return false;
        }
        this->mCorComponent->YieldReturn();
        return redisTask->GetErrorCode() == XCode::Successful;
    }

    bool SceneRedisComponent::DelValue(const std::string &key)
    {
		RedisTask * redisTask = this->CreateTask("DEL", key);
        if (!this->mTaskManager->StartTask(redisTask))
        {
            return false;
        }
        this->mCorComponent->YieldReturn();
        return redisTask->GetErrorCode() == XCode::Successful;
    }

    bool SceneRedisComponent::DelValue(const std::string &tab, const std::string &key)
    {
		RedisTask * redisTask = this->CreateTask("HDEL", tab, key);
        if (!this->mTaskManager->StartTask(redisTask))
        {
            return false;
        }
        this->mCorComponent->YieldReturn();
        return redisTask->GetErrorCode() == XCode::Successful;
    }

    bool SceneRedisComponent::SetValue(const std::string &key, const std::string &value, int second)
    {
		RedisTask * redisTask = this->CreateTask("EXPIRE", key, second);
        if (!this->mTaskManager->StartTask(redisTask))
        {
            return false;
        }
        this->mCorComponent->YieldReturn();
        return redisTask->GetErrorCode() == XCode::Successful;
    }

    bool SceneRedisComponent::SetValue(const std::string &key, const std::string &value)
    {
		RedisTask * redisTask = this->CreateTask("SET", key, value);
        if (!this->mTaskManager->StartTask(redisTask))
        {
            return false;
        }
        this->mCorComponent->YieldReturn();
        return redisTask->GetErrorCode() == XCode::Successful;
    }

    bool SceneRedisComponent::SetValue(const std::string &tab, const std::string &key, const std::string &value)
    {
		RedisTask * redisTask = this->CreateTask("HSET", tab, key, value);
        if (!this->mTaskManager->StartTask(redisTask))
        {
            return false;
        }
        this->mCorComponent->YieldReturn();
        return redisTask->GetErrorCode() == XCode::Successful;

    }

    bool SceneRedisComponent::SetValue(const std::string &tab, const std::string &key, const shared_ptr<Message> value)
    {
        std::string serializeData;
        if (!value->SerializePartialToString(&serializeData))
        {
            return false;
        }
        return this->SetValue(tab, key, serializeData);
    }

    bool SceneRedisComponent::GetValue(const std::string &key, std::string &value)
    {
		RedisTask * redisTask = this->CreateTask("GET", key);
        if (!this->mTaskManager->StartTask(redisTask))
        {
            return false;
        }
        return redisTask->GetOnceData(value);
    }

    bool SceneRedisComponent::GetValue(const std::string &tab, const std::string &key, std::string &value)
    {
		RedisTask * redisTask = this->CreateTask("HGET", tab, key);
        if (!this->mTaskManager->StartTask(redisTask))
        {
            return false;
        }
        return redisTask->GetOnceData(value);
    }

    bool SceneRedisComponent::GetValue(const std::string &tab, const std::string &key, shared_ptr<Message> value)
    {
        std::string message;
        if (!this->GetValue(tab, key, message))
        {
            return false;
        }
        return value->ParseFromString(message);
    }
}
