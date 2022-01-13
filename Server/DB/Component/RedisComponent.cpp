#include"RedisComponent.h"

#include"Core/App.h"
#include"Util/StringHelper.h"
#include"Util/FileHelper.h"
#include"Util/DirectoryHelper.h"
#include"Script/ClassProxyHelper.h"
#include"Scene/RpcConfigComponent.h"
#include"Scene/ThreadPoolComponent.h"

namespace GameKeeper
{
    bool RedisComponent::Awake()
    {
        std::string path;
        this->mRedisPort = 0;
        this->mPubSubThread = nullptr;
        this->mPubSubContext = nullptr;
        const ServerConfig &config = App::Get().GetConfig();

        LOG_CHECK_RET_FALSE(config.GetValue("redis", "ip", this->mRedisIp));
        LOG_CHECK_RET_FALSE(config.GetValue("redis", "port", this->mRedisPort));
        return true;
    }

    std::shared_ptr<RedisResponse>
    RedisComponent::Call(const std::string &tab, const std::string &func, std::vector<std::string> &args)
    {
        std::string script;
        if(!this->GetLuaScript(fmt::format("{0}.lua", tab), script))
        {
            LOG_ERROR("not find redis script ", fmt::format("{0}.lua", tab));
            return nullptr;
        }
        std::shared_ptr<RedisTaskSource> redisTask
                = std::make_shared<RedisTaskSource>("EVALSHA");
        redisTask->AddCommand(script);
        redisTask->AddCommand((int)args.size() + 1);
        redisTask->AddCommand(func);
        for(const std::string & val : args)
        {
            redisTask->AddCommand(val);
        }
        auto response = redisTask->Await();
        if(response->GetCode() != XCode::Successful)
        {
            LOG_ERROR(response->GetError());
        }
        return response;
    }

    bool RedisComponent::LoadLuaScript(const std::string &path)
    {
        std::string content;
        if(!Helper::File::ReadTxtFile(path, content))
        {
            return false;
        }
        auto response = this->Invoke("script", "load", content);
        if(response->GetCode() != XCode::Successful)
        {
            LOG_ERROR(response->GetError());
            return false;
        }
        std::string fileName, luaCommand;
        response->GetValue(luaCommand);
        Helper::Directory::GetDirAndFileName(path, content, fileName);
        this->mLuaCommandMap.emplace(fileName,  luaCommand);
        return true;
    }

    bool RedisComponent::GetLuaScript(const std::string &file, std::string &command)
    {
        auto iter = this->mLuaCommandMap.find(file);
        if(iter != this->mLuaCommandMap.end())
        {
            command = iter->second;
            return true;
        }
        return false;
    }

    void RedisComponent::OnStart()
    {
        std::string path;
        std::vector<std::string> luaFiles;
        App::Get().GetConfig().GetValue("redis", "lua", path);
        Helper::Directory::GetFilePaths(path, "*.lua", luaFiles);

        for(const std::string & file : luaFiles)
        {
            LOG_CHECK_RET(this->LoadLuaScript(file));
            LOG_INFO("load redis script ", file, " successful");
        }

        std::vector<std::string> services;
        this->GetComponent<RpcConfigComponent>()->GetServices(services);
        std::shared_ptr<RedisResponse> response = this->Call("Service", "Push", services);
        if(response->GetCode() == XCode::Successful)
        {
            LOG_WARN("push new service count = ", response->GetNumber());
        }
    }

    void RedisComponent::StartPubSub()
    {

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
        this->mPubSubContext = this->ConnectRedis(second);
        this->mPubSubThread = new std::thread(std::bind(&RedisComponent::StartPubSub, this));
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
            LOG_FATAL("connect redis fail =>[", mRedisIp, ':', mRedisPort, "] error = ", pRedisContext->errstr);
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
