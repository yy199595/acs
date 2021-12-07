#include"RedisComponent.h"
#include<Util/StringHelper.h>
#include<Scene/TaskPoolComponent.h>
#include<Coroutine/CoroutineComponent.h>
#include<Script/ClassProxyHelper.h>
#include<Core/App.h>
#include"Other/ElapsedTimer.h"
namespace GameKeeper
{
    RedisComponent::RedisComponent()
    {
        this->mRedisPort = 0;
        this->mTaskManager = nullptr;
        this->mCorComponent = nullptr;
        this->mLastOperatorTime = TimeHelper::GetSecTimeStamp();
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

    bool RedisComponent::Awake()
    {
        const ServerConfig &config = App::Get().GetConfig();
		LOG_CHECK_RET_FALSE(config.GetValue("Redis", "ip", this->mRedisIp));
		LOG_CHECK_RET_FALSE(config.GetValue("Redis", "port", this->mRedisPort));
        LOG_CHECK_RET_FALSE(this->mTaskManager = this->GetComponent<TaskPoolComponent>());
        LOG_CHECK_RET_FALSE(this->mCorComponent = this->GetComponent<CoroutineComponent>());


        int second = 3;
		config.GetValue("Redis", "timeout", second);
        const std::vector<TaskThread *> &threads = this->mTaskManager->GetThreads();
       
        for (TaskThread *taskThread: threads)
        {
            redisContext *redisSocket = this->ConnectRedis(second);
            if (redisSocket == nullptr)
            {
                return false;
            }
            this->mRedisContextMap.emplace(taskThread->GetThreadId(), redisSocket);
            LOG_DEBUG("connect redis successful [" << mRedisIp << ":" << mRedisPort << "]");
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
            LOG_FATAL(
                    "connect redis fail " << mRedisIp << ":" << mRedisPort << " error:" << pRedisContext->errstr);
            return nullptr;
        }
        std::string redisPasswd;
        if (config.GetValue("Redis", "passwd", redisPasswd) && !redisPasswd.empty())
        {
            auto *reply = (redisReply *) redisCommand(pRedisContext, "auth %s", redisPasswd.c_str());
            if (reply == nullptr || reply->type == REDIS_REPLY_ERROR)
            {
                LOG_ERROR("redis Authentication failed " << reply->str);
                return nullptr;
            }
            freeReplyObject(reply);
        }
        return pRedisContext;
    }

    void RedisComponent::Start()
    {
        std::string json;
        db::UserAccountData userAccountData;
        userAccountData.set_account("646586122@qq.com");
        userAccountData.set_token("sjfjsdiofjiowejfow");
        userAccountData.set_passwd("199595yjz.");
        userAccountData.set_lastlogintime(TimeHelper::GetSecTimeStamp());
        userAccountData.set_userid(1995);

        this->ClearAllData();

    }
    std::shared_ptr<RedisResponse> RedisComponent::StartTask(std::shared_ptr<RedisTaskProxy> redisTask)
    {
        this->mTaskManager->StartTask(redisTask.get());
        this->mLastOperatorTime = TimeHelper::GetSecTimeStamp();

        this->mCorComponent->WaitForYield();
        auto redisResponse = redisTask->GetResponse();
        if(redisResponse->HasError())
        {
            LOG_ERROR(redisResponse->GetError());
        }
        return redisResponse;
    }

    long long RedisComponent::AddCounter(const string &key)
    {
       return this->Invoke("INCR", key)->GetNumber();
    }
}
