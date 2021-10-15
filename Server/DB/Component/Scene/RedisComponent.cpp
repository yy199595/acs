#include"RedisComponent.h"
#include<Util/StringHelper.h>
#include<Scene/TaskPoolComponent.h>
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
            RedisSocket *socket = iter->second;
            redisFree(socket);
            this->mRedisContextMap.erase(iter);
            return true;
        }
        return false;
    }

    void RedisComponent::ClearAllData()
    {
        auto iter = this->mRedisContextMap.begin();
        for(; iter!=this->mRedisContextMap.end();iter++)
        {
            RedisSocket * c = iter->second;
            redisCommand(c, "FLUSHALL");
            return;
        }
    }

    RedisSocket *RedisComponent::GetRedisSocket()
    {
        auto id = std::this_thread::get_id();
        auto iter = this->mRedisContextMap.find(id);
        return iter != this->mRedisContextMap.end() ? iter->second : nullptr;
    }

    bool RedisComponent::Awake()
    {
        ServerConfig &config = App::Get().GetConfig();
        SayNoAssertRetFalse_F(this->mTaskManager = this->GetComponent<TaskPoolComponent>());
        SayNoAssertRetFalse_F(this->mCorComponent = this->GetComponent<CoroutineComponent>());


        SayNoAssertRetFalse_F(config.GetValue("Redis", "ip", this->mRedisIp));
        SayNoAssertRetFalse_F(config.GetValue("Redis", "port", this->mRedisPort));

        int second = 3;
        const std::vector<TaskThread *> &threads = this->mTaskManager->GetThreads();
        config.GetValue("Redis", "timeout", second);

        for (TaskThread *taskThread: threads)
        {
            redisContext *redisSocket = this->ConnectRedis(second);
            if (redisSocket == nullptr)
            {
                return false;
            }
            this->mRedisContextMap.emplace(taskThread->GetThreadId(), redisSocket);
            SayNoDebugLog("connect redis successful [" << mRedisIp << ":" << mRedisPort << "]");
        }
        return true;
    }

    redisContext *RedisComponent::ConnectRedis(int timeout)
    {
        struct timeval tv;
        tv.tv_sec = 3;
        tv.tv_usec = tv.tv_sec * timeout * 1000;
        ServerConfig &config = App::Get().GetConfig();
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
            void *p = redisCommand(pRedisContext, "auth %s", redisPasswd.c_str());
            redisReply *reply = (redisReply *) redisCommand(pRedisContext, "auth %s", redisPasswd.c_str());
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
        RedisTask redisTask("GET");
        redisTask.InitCommand(key);
        if (this->mTaskManager->StartTask(&redisTask) == 0)
        {
            return false;
        }
        this->mCorComponent->YieldReturn();
        return redisTask.GetErrorCode() == XCode::Successful;
    }

    bool RedisComponent::HasValue(const std::string &tab, const std::string &key)
    {
        RedisTask redisTask("HEXISTS");
        redisTask.InitCommand(tab, key);
        if (!this->mTaskManager->StartTask(&redisTask))
        {
            return false;
        }
        this->mCorComponent->YieldReturn();
        return redisTask.GetErrorCode() == XCode::Successful;
    }

    bool RedisComponent::DelValue(const std::string &key)
    {
        RedisTask redisTask("DEL");
        redisTask.InitCommand(key);
        if (!this->mTaskManager->StartTask(&redisTask))
        {
            return false;
        }
        this->mCorComponent->YieldReturn();
        return redisTask.GetErrorCode() == XCode::Successful;
    }

    bool RedisComponent::DelValue(const std::string &tab, const std::string &key)
    {
        RedisTask redisTask("HDEL");
        redisTask.InitCommand(tab, key);
        if (!this->mTaskManager->StartTask(&redisTask))
        {
            return false;
        }
        this->mCorComponent->YieldReturn();
        return redisTask.GetErrorCode() == XCode::Successful;
    }

    bool RedisComponent::SetValue(const std::string &key, const std::string &value, int second)
    {
        RedisTask redisTask("EXPIRE");
        redisTask.InitCommand(value, second);
        if (!this->mTaskManager->StartTask(&redisTask))
        {
            return false;
        }
        this->mCorComponent->YieldReturn();
        return redisTask.GetErrorCode() == XCode::Successful;
    }

    bool RedisComponent::SetValue(const std::string &key, const std::string &value)
    {
        RedisTask redisTask("SET");
        redisTask.InitCommand(key, value);
        if (!this->mTaskManager->StartTask(&redisTask))
        {
            return false;
        }
        this->mCorComponent->YieldReturn();
        return redisTask.GetErrorCode() == XCode::Successful;
    }

    bool RedisComponent::SetValue(const std::string &tab, const std::string &key, const std::string &value)
    {
        RedisTask redisTask("HSET");
        redisTask.InitCommand(tab, key, value);
        if (!this->mTaskManager->StartTask(&redisTask))
        {
            return false;
        }
        this->mCorComponent->YieldReturn();
        return redisTask.GetErrorCode() == XCode::Successful;
    }

    bool RedisComponent::SetJsonValue(const std::string & key, const Message & value)
    {
        std::string jsonData;
        if(!util::MessageToJsonString(value, & jsonData).ok())
        {
            return false;
        }
        return this->SetValue(key, jsonData);
    }

    bool RedisComponent::SetJsonValue(const std::string &tab, const std::string &key, const Message & value)
    {
        std::string jsonData;
        if (!util::MessageToJsonString(value, &jsonData).ok())
        {
            return false;
        }
        return this->SetValue(tab, key, jsonData);
    }

    bool RedisComponent::SetValue(const std::string & key, const Message & value)
    {
        std::string valueData;
        if(!value.SerializeToString(&valueData))
        {
            return false;
        }
        return this->SetValue(key, valueData);
    }

    bool RedisComponent::SetValue(const std::string &tab, const std::string &key, const Message &value)
    {
        std::string valueData;
        if(!value.SerializeToString(&valueData))
        {
            return false;
        }
        return this->SetValue(tab, key, valueData);
    }

    bool RedisComponent::GetValue(const std::string &key, std::string &value)
    {
        RedisTask redisTask("GET");
        redisTask.InitCommand(key);
        if (!this->mTaskManager->StartTask(&redisTask))
        {
            return false;
        }
        this->mCorComponent->YieldReturn();
        return redisTask.GetOnceData(value);
    }

    bool RedisComponent::GetValue(const std::string &tab, const std::string &key, std::string &value)
    {
        RedisTask redisTask("HGET");
        redisTask.InitCommand(tab, key);
        if (!this->mTaskManager->StartTask(&redisTask))
        {
            return false;
        }
        this->mCorComponent->YieldReturn();
        return redisTask.GetOnceData(value);
    }

    bool RedisComponent::GetValue(const std::string &tab, const std::string &key, Message &value)
    {
        std::string message;
        if (!this->GetValue(tab, key, message))
        {
            return false;
        }
        return value.ParseFromString(message);
    }

    bool RedisComponent::GetJsonValue(const std::string & key, Message & value)
    {
        std::string jsonValue;
        if(!this->GetValue(key, jsonValue))
        {
            return false;
        }
        return util::JsonStringToMessage(jsonValue, &value).ok();
    }

    bool RedisComponent::GetJsonValue(const std::string &tab, const std::string &key, Message & value)
    {
        std::string jsonValue;
        if (!this->GetValue(tab, key, jsonValue))
        {
            return false;
        }
        return util::JsonStringToMessage(jsonValue, &value).ok();
    }

    bool RedisComponent::AddToSet(const std::string & set, const std::string &member)
    {
        RedisTask redisTask("SADD");
        redisTask.InitCommand(set, member);
        if (!this->mTaskManager->StartTask(&redisTask))
        {
            return false;
        }
        this->mCorComponent->YieldReturn();
        return redisTask.GetErrorCode() == XCode::Successful;
    }

    bool RedisComponent::DelFromSet(const std::string & set, const std::string & member)
    {
        RedisTask redisTask("SREM");
        redisTask.InitCommand(set, member);
        if (!this->mTaskManager->StartTask(&redisTask))
        {
            return false;
        }
        this->mCorComponent->YieldReturn();
        return redisTask.GetErrorCode() == XCode::Successful;
    }
}
