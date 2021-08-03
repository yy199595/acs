#include"RedisManager.h"
#include<Util/NumberHelper.h>
#include<Util/StringHelper.h>
#include<Manager/ThreadTaskManager.h>
#include<RedisClient/RedisTaskAction.h>
#include<Coroutine/CoroutineManager.h>
#include<Script/ClassProxyHelper.h>

namespace Sentry
{
    RedisManager::RedisManager()
    {
        this->mRedisPort = 0;
    }

    RedisSocket *RedisManager::GetRedisSocket(long long id)
    {
        auto iter = this->mRedisContextMap.find(id);
        return iter != this->mRedisContextMap.end() ? iter->second : nullptr;
    }

    bool RedisManager::OnInit()
    {
        std::string redisAddress;

        SayNoAssertRetFalse_F(this->mCorManager = this->GetManager<CoroutineManager>());
        SayNoAssertRetFalse_F(this->mTaskManager = this->GetManager<ThreadTaskManager>());

        SayNoAssertRetFalse_F(this->GetConfig().GetValue("RedisAddress", redisAddress));
        SayNoAssertRetFalse_F(StringHelper::ParseIpAddress(redisAddress, mRedisIp, mRedisPort));

        struct timeval tv;
        tv.tv_sec = 3;
        tv.tv_usec = tv.tv_sec * 1000;
        if (this->GetConfig().GetValue("RedisTimeout", tv.tv_sec))
        {
            tv.tv_usec = tv.tv_sec * 1000;
        }

        int count = this->mTaskManager->GetThreadCount();
        if (count == 0)
        {
            return false;
        }
        for (int index = 0; index < count; index++)
        {
            redisContext *pRedisContext = redisConnectWithTimeout(mRedisIp.c_str(), mRedisPort, tv);
            if (pRedisContext->err != 0)
            {
                SayNoDebugFatal(
                        "connect redis fail " << mRedisIp << ":" << mRedisPort << " error:" << pRedisContext->errstr);
                return false;
            }
            std::string redisPasswd;
            if (this->GetConfig().GetValue("RedisPasswd", redisPasswd) && !redisPasswd.empty())
            {
                redisReply *reply = (redisReply *) redisCommand(pRedisContext, "AUTH %s", redisPasswd.c_str());
                if (reply == nullptr || reply->type == REDIS_REPLY_ERROR)
                {
                    SayNoDebugError("redis Authentication failed " << reply->str);
                    return false;
                }
                freeReplyObject(reply);
            }

            this->mRedisContextMap.emplace(index, pRedisContext);
        }
        SayNoDebugLog("connect redis successful [" << mRedisIp << ":" << mRedisPort << "]  [ count:" << count << "]");

        return true;
    }

    void RedisManager::OnInitComplete()
    {

    }

    bool RedisManager::HasValue(const std::string &key)
    {
        RedisSharedTask redisTask = this->CreateTask("GET", key);

        if (this->mTaskManager->StartInvokeTask(redisTask) == 0)
        {
            return false;
        }
        this->mCorManager->YieldReturn();
        return redisTask->GetErrorCode() == XCode::Successful;
    }

    bool RedisManager::HasValue(const std::string &tab, const std::string &key)
    {
        RedisSharedTask redisTask = this->CreateTask("HEXISTS", tab, key);
        if (!this->mTaskManager->StartInvokeTask(redisTask))
        {
            return false;
        }
        this->mCorManager->YieldReturn();
        return redisTask->GetErrorCode() == XCode::Successful;
    }

    bool RedisManager::DelValue(const std::string &key)
    {
        RedisSharedTask redisTask = this->CreateTask("DEL", key);
        if (!this->mTaskManager->StartInvokeTask(redisTask))
        {
            return false;
        }
        this->mCorManager->YieldReturn();
        return redisTask->GetErrorCode() == XCode::Successful;
    }

    bool RedisManager::DelValue(const std::string &tab, const std::string &key)
    {
        RedisSharedTask redisTask = this->CreateTask("HDEL", tab, key);
        if (!this->mTaskManager->StartInvokeTask(redisTask))
        {
            return false;
        }
        this->mCorManager->YieldReturn();
        return redisTask->GetErrorCode() == XCode::Successful;
    }

    bool RedisManager::SetValue(const std::string &key, const std::string &value, int second)
    {
        RedisSharedTask redisTask = this->CreateTask("EXPIRE", key, second);
        if (!this->mTaskManager->StartInvokeTask(redisTask))
        {
            return false;
        }
        this->mCorManager->YieldReturn();
        return redisTask->GetErrorCode() == XCode::Successful;
    }

    bool RedisManager::SetValue(const std::string &key, const std::string &value)
    {
        RedisSharedTask redisTask = this->CreateTask("SET", key, value);
        if (!this->mTaskManager->StartInvokeTask(redisTask))
        {
            return false;
        }
        this->mCorManager->YieldReturn();
        return redisTask->GetErrorCode() == XCode::Successful;
    }

    bool RedisManager::SetValue(const std::string &tab, const std::string &key, const std::string &value)
    {
        RedisSharedTask redisTask = this->CreateTask("HSET", tab, key, value);
        if (!this->mTaskManager->StartInvokeTask(redisTask))
        {
            return false;
        }
        this->mCorManager->YieldReturn();
        return redisTask->GetErrorCode() == XCode::Successful;

    }

    bool RedisManager::SetValue(const std::string &tab, const std::string &key, const shared_ptr<Message> value)
    {
        std::string serializeData;
        if (!value->SerializePartialToString(&serializeData))
        {
            return false;
        }
        return this->SetValue(tab, key, serializeData);
    }

    bool RedisManager::GetValue(const std::string &key, std::string &value)
    {
        RedisSharedTask redisTask = this->CreateTask("GET", key);
        if (!this->mTaskManager->StartInvokeTask(redisTask))
        {
            return false;
        }
        return redisTask->GetOnceData(value);
    }

    bool RedisManager::GetValue(const std::string &tab, const std::string &key, std::string &value)
    {
        RedisSharedTask redisTask = this->CreateTask("HGET", tab, key);
        if (!this->mTaskManager->StartInvokeTask(redisTask))
        {
            return false;
        }
        return redisTask->GetOnceData(value);
    }

    bool RedisManager::GetValue(const std::string &tab, const std::string &key, shared_ptr<Message> value)
    {
        std::string message;
        if (!this->GetValue(tab, key, message))
        {
            return false;
        }
        return value->ParseFromString(message);
    }
}
