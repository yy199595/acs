#include"RedisComponent.h"

#include"Core/App.h"
#include"Util/StringHelper.h"
#include"Script/ClassProxyHelper.h"
#include"Component/Scene/ThreadPoolComponent.h"

namespace GameKeeper
{
    bool RedisComponent::Awake()
    {
        this->mRedisPort = 0;
        const ServerConfig &config = App::Get().GetConfig();
        LOG_CHECK_RET_FALSE(config.GetValue("Redis", "ip", this->mRedisIp));
        LOG_CHECK_RET_FALSE(config.GetValue("Redis", "port", this->mRedisPort));
        return true;
    }

    void RedisComponent::OnStart()
    {
//        for (int index = 0; index < 10; index++)
//        {
//            long long num = this->AddCounter("UserIdCounter");
//            LOG_ERROR("number = " << num);
//        }
    }

    bool RedisComponent::CloseRedisSocket()
    {
        auto id = std::this_thread::get_id();
        auto iter = this->mRedisContextMap.find(id);
        if (iter != this->mRedisContextMap.end())
        {
            redisFree(iter->second);
            this->mRedisContextMap.erase(iter);
            return true;
        }
        return false;
    }

    void RedisComponent::ClearAllData()
    {
        auto iter = this->mRedisContextMap.begin();
		for (; iter != this->mRedisContextMap.end(); iter++)
		{
			redisCommand(iter->second, "FLUSHALL");
			return;
		}
    }

    RedisSocket *RedisComponent::GetRedisSocket()
    {
        auto id = std::this_thread::get_id();
        auto iter = this->mRedisContextMap.find(id);
        return iter != this->mRedisContextMap.end() ? iter->second : nullptr;
    }

    bool RedisComponent::LateAwake()
    {
        int second = 3;
        App::Get().GetConfig().GetValue("Redis", "timeout", second);
        auto threadComponent = this->GetComponent<ThreadPoolComponent>();
        const std::vector<TaskThread *> &threads = threadComponent->GetThreads();
       
        for (TaskThread *taskThread: threads)
        {
            redisContext *redisSocket = this->ConnectRedis(second);
            if (redisSocket == nullptr)
            {
                return false;
            }
            this->mRedisContextMap.emplace(taskThread->GetThreadId(), redisSocket);
            LOG_DEBUG("connect redis successful [", mRedisIp, ':', mRedisPort, "]");
        }
        return true;
    }

    redisContext *RedisComponent::ConnectRedis(int timeout)
    {
        struct timeval tv{};
        tv.tv_sec = 3;
        tv.tv_usec = tv.tv_sec * timeout * 1000;
        const ServerConfig &config = App::Get().GetConfig();
        redisContext *pRedisContext = redisConnectWithTimeout(mRedisIp.c_str(), mRedisPort, tv);
        if (pRedisContext->err != 0)
        {
            LOG_FATAL("connect redis fail", mRedisIp, ':', mRedisPort, "error = ", pRedisContext->errstr);
            return nullptr;
        }
        std::string redisPasswd;
        if (config.GetValue("Redis", "passwd", redisPasswd) && !redisPasswd.empty())
        {
            auto *reply = (redisReply *) redisCommand(pRedisContext, "auth %s", redisPasswd.c_str());
            if (reply == nullptr || reply->type == REDIS_REPLY_ERROR)
            {
                LOG_ERROR("redis Authentication failed {0}", reply->str);
                return nullptr;
            }
            freeReplyObject(reply);
        }
        return pRedisContext;
    }

    long long RedisComponent::AddCounter(const string &key)
    {
       return this->Invoke("INCR", key)->GetNumber();
    }
}
