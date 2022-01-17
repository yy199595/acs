#include"RedisComponent.h"

#include"Core/App.h"
#include"Util/StringHelper.h"
#include"Util/FileHelper.h"
#include"Util/DirectoryHelper.h"
#include"Script/ClassProxyHelper.h"
#include"Scene/RpcConfigComponent.h"
#include"Scene/ThreadPoolComponent.h"
#include"RedisClient/NetWork/RedisClient.h"
namespace GameKeeper
{
    bool RedisComponent::Awake()
    {
        std::string path;
        const ServerConfig &config = App::Get().GetConfig();
        config.GetValue("redis", "lua", this->mRedisConfig.mLuaFilePath);
        config.GetValue("redis", "passwd", this->mRedisConfig.mPassword);
        LOG_CHECK_RET_FALSE(config.GetValue("redis", "ip", this->mRedisConfig.mIp));
        LOG_CHECK_RET_FALSE(config.GetValue("redis", "port", this->mRedisConfig.mPort));
        return true;
    }

    std::shared_ptr<RedisCmdResponse>
    RedisComponent::Call(const std::string &tab, const std::string &func, std::vector<std::string> &args)
    {
        std::string script;
        if(!this->GetLuaScript(fmt::format("{0}.lua", tab), script))
        {
            LOG_ERROR("not find redis script ", fmt::format("{0}.lua", tab));
            return nullptr;
        }
        std::shared_ptr<RedisCmdRequest> redisCmdRequest(new RedisCmdRequest("EVALSHA"));
        redisCmdRequest->InitParamater(script, (int)args.size() + 1, func);
        for(const std::string & val : args)
        {
            redisCmdRequest->AddParamater(val);
        }
        return this->InvokeCommand(redisCmdRequest);
    }

    std::shared_ptr<RedisCmdResponse> RedisComponent::InvokeCommand(std::shared_ptr<RedisCmdRequest> request)
    {
       std::shared_ptr<RedisClient> redisClient =  this->AllotRedisClient();
#ifdef __DEBUG__
        ElapsedTimer elapsedTimer;
#endif
        if(redisClient == nullptr)
        {
            LOG_ERROR("allot redis client failure");
            return nullptr;
        }
        auto response = redisClient->InvokeCommand(request)->Await();
#ifdef __DEBUG__
        LOG_INFO("invoke redis command use time : ", elapsedTimer.GetMs(), "ms");
#endif
        this->mWaitRedisClient.emplace(redisClient);
        return response;
    }

    bool RedisComponent::LoadLuaScript(const std::string &path)
    {
        std::string content;
        if(!Helper::File::ReadTxtFile(path, content))
        {
            return false;
        }
        auto response = this->InvokeCommand("script", "load", content);
        if(response->HasError())
        {
            LOG_ERROR(response->GetValue());
            return false;
        }
        std::string fileName;
        Helper::Directory::GetDirAndFileName(path, content, fileName);
        LOG_WARN(fileName, "  ", response->GetValue());
        this->mLuaCommandMap.emplace(fileName,  response->GetValue());
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

        this->mSubRedisClient = this->MakeRedisClient("Subscribe");
        if(this->mSubRedisClient != nullptr)
        {
            LOG_DEBUG("subscribe redis client connect successful");
            this->mTaskComponent->Start(&RedisComponent::StartPubSub, this);
        }
        for (const std::string &file: luaFiles)
        {
            LOG_CHECK_RET(this->LoadLuaScript(file));
            LOG_INFO("load redis script ", file, " successful");
        }
    }

    std::shared_ptr<RedisClient> RedisComponent::MakeRedisClient(const std::string & name)
    {
        NetWorkThread &workThread = this->mThreadComponent->AllocateNetThread();
        auto socketProxy = std::make_shared<SocketProxy>(workThread, name);
        auto redisCommandClient = std::make_shared<RedisClient>(socketProxy);

        for (size_t index = 0; index < 3; index++)
        {
            if (redisCommandClient->ConnectAsync(this->mRedisConfig.mIp, this->mRedisConfig.mPort)->Await())
            {
                this->mRedisCmdClients.emplace_back(redisCommandClient);
                if(!this->mRedisConfig.mPassword.empty())
                {
                    std::shared_ptr<RedisCmdRequest> request(new RedisCmdRequest("AUTH"));
                    request->AddParamater(this->mRedisConfig.mPassword);
                    auto response = redisCommandClient->InvokeCommand(request)->Await();
                    if(!response->IsOk())
                    {
                        LOG_ERROR("auth redis passwork error : ", this->mRedisConfig.mPassword);
                        return nullptr;
                    }
                }
                return redisCommandClient;
            }
        }
    }

    std::shared_ptr<RedisClient> RedisComponent::AllotRedisClient()
    {
        if(!this->mWaitRedisClient.empty())
        {
            auto redisClinet = this->mWaitRedisClient.front();
            this->mWaitRedisClient.pop();
            if(!redisClinet->IsOpen())
            {
                const std::string & ip = this->mRedisConfig.mIp;
                unsigned short port = this->mRedisConfig.mPort;
                if(!redisClinet->ConnectAsync(ip, port)->Await())
                {
                    return nullptr;
                }
                if(!this->mRedisConfig.mPassword.empty())
                {
                    std::shared_ptr<RedisCmdRequest> request(new RedisCmdRequest("AUTH"));
                    request->AddParamater(this->mRedisConfig.mPassword);
                    auto response = redisClinet->InvokeCommand(request)->Await();
                    if(!response->IsOk())
                    {
                        LOG_ERROR("auth redis passwork error : ", this->mRedisConfig.mPassword);
                        return nullptr;
                    }
                }
            }
            return redisClinet;
        }
        return this->MakeRedisClient("Command");
    }

    void RedisComponent::StartPubSub()
    {

        auto response1 = this->InvokeCommand("AUTH", "199595yjz.");
        auto response2 = this->InvokeCommand("SADD", "yjz", 11223, 445345, "sajsaiojrioew");

        std::shared_ptr<RedisCmdRequest> request(new RedisCmdRequest("SUBSCRIBE"));
        request->AddParamater("chat");
        this->mSubRedisClient->InvokeCommand(request)->Await();
        while (this->mSubRedisClient)
        {
            if (!this->mSubRedisClient->IsOpen())
            {
                int count = 0;
                const std::string & ip = this->mRedisConfig.mIp;
                unsigned short port = this->mRedisConfig.mPort;
                while (!this->mSubRedisClient->ConnectAsync(ip, port)->Await())
                {
                    LOG_ERROR("connect redis [", ip, ':', port, "] failure count = ", count++);
                    this->mTaskComponent->Sleep(3000);
                }
                if(!this->mRedisConfig.mPassword.empty())
                {
                    std::shared_ptr<RedisCmdRequest> request(new RedisCmdRequest("AUTH"));
                    request->AddParamater(this->mRedisConfig.mPassword);
                    auto response = this->mSubRedisClient->InvokeCommand(request)->Await();
                    if(response == nullptr || response->IsOk())
                    {
                        LOG_FATAL("auth redis passwd error", this->mRedisConfig.mPassword);
                        return;
                    }
                }
            }
            auto redisClientTask = this->mSubRedisClient->WaitRedisMessageResponse();

            auto redisResponse = redisClientTask->Await();

            if(!redisResponse->HasError())
            {
                for (size_t index = 0; index < redisResponse->GetArraySize(); index++)
                {
                    LOG_ERROR(redisResponse->GetValue(index));
                }
            }
        }
    }

    bool RedisComponent::Ping(std::shared_ptr<RedisClient> redisClient)
    {
        std::shared_ptr<RedisCmdRequest> request(new RedisCmdRequest("PING"));
        auto response = this->mSubRedisClient->InvokeCommand(request)->Await();
        if(response == nullptr)
        {
            LOG_ERROR("redis net error");
            return false;
        }
        return true;
    }

    bool RedisComponent::LateAwake()
    {
        int second = 3;
        this->mTaskComponent = this->GetComponent<TaskComponent>();
        this->mThreadComponent = this->GetComponent<ThreadPoolComponent>();
        App::Get().GetConfig().GetValue("Redis", "timeout", second);
        return true;
    }


    long long RedisComponent::AddCounter(const string &key)
    {
       return this->InvokeCommand("INCR", key)->GetNumber();
    }
}
