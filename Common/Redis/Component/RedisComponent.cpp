//
// Created by mac on 2022/5/18.
//

#include"RedisComponent.h"
#include"File/FileHelper.h"
#include"Timer/ElapsedTimer.h"
#include"Component/NetThreadComponent.h"

namespace Sentry
{
    bool RedisLuaResponse::ParseJson(const std::string &json)
    {
        if(this->mJson.ParseJson(json))
        {
            return true;
        }
        return false;
    }

    bool RedisLuaResponse::GetResult()
    {
        bool res = false;
        return this->mJson.GetMember("res", res) && res;
    }
}

namespace Sentry
{
	bool RedisComponent::LoadConfig()
	{
        const ServerConfig * localServerConfig = ServerConfig::Inst();
        const rapidjson::Value * jsonValue = localServerConfig->GetJsonValue("redis");

        if(jsonValue == nullptr || !jsonValue->IsObject())
        {
            return false;
        }
        auto iter = jsonValue->MemberBegin();
        for(; iter != jsonValue->MemberEnd(); iter++)
        {
            const char * name = iter->name.GetString();
            if(!this->ParseConfig(name, iter->value))
            {
                return false;
            }
        }
        this->mNetComponent = this->GetComponent<NetThreadComponent>();
        return true;
	}

    bool RedisComponent::StartConnectRedis()
    {
        if(!this->LoadConfig())
        {
            return false;
        }
        auto iter = this->mConfigs.begin();
        for(; iter != this->mConfigs.end(); iter++)
        {
            if(!this->OnInitRedisClient(iter->second))
            {
                return false;
            }
        }
        return true;
    }

    bool RedisComponent::ParseConfig(const char *name, const rapidjson::Value &jsonData)
    {
        auto iter = this->mConfigs.find(name);
        if (iter != this->mConfigs.end())
        {
            return true;
        }

        LOG_CHECK_RET_FALSE(jsonData.HasMember("ip"));
        LOG_CHECK_RET_FALSE(jsonData.HasMember("port"));
        LOG_CHECK_RET_FALSE(jsonData.HasMember("index"));
        LOG_CHECK_RET_FALSE(jsonData.HasMember("count"));

        RedisConfig redisConfig;
        redisConfig.Name = name;
        redisConfig.FreeClient = 30;
        redisConfig.Ip = jsonData["ip"].GetString();
        redisConfig.Port = jsonData["port"].GetInt();
        redisConfig.Index = jsonData["index"].GetInt();
        redisConfig.Count = jsonData["count"].GetInt();
        if (jsonData.HasMember("free"))
        {
            redisConfig.FreeClient = jsonData["free"].GetInt();
        }
        if (jsonData.HasMember("passwd"))
        {
            redisConfig.Password = jsonData["passwd"].GetString();
        }
        if (jsonData.HasMember("scripts") && jsonData["scripts"].IsArray())
        {
            for (int index = 0; index < jsonData["scripts"].Size(); index++)
            {
                std::string lua(jsonData["scripts"][index].GetString());
                redisConfig.LuaFiles.emplace_back(lua);
            }
        }

        if (jsonData.HasMember("channels") && jsonData["channels"].IsArray())
        {
            for (int index = 0; index < jsonData["channels"].Size(); index++)
            {
                std::string lua(jsonData["channels"][index].GetString());
                redisConfig.Channels.emplace_back(lua);
            }
        }
        redisConfig.Address = fmt::format("{0}:{1}", redisConfig.Ip, redisConfig.Port);
        this->mConfigs.emplace(name, redisConfig);
        return true;
    }

    TcpRedisClient * RedisComponent::MakeRedisClient(const RedisConfig & config)
	{
        std::shared_ptr<SocketProxy> socketProxy = this->mNetComponent->CreateSocket();
        if(socketProxy == nullptr)
        {
            return nullptr;
        }
        socketProxy->Init(config.Ip, config.Port);
        std::shared_ptr<TcpRedisClient> redisClient =
            std::make_shared<TcpRedisClient>(socketProxy, config, this);
        this->mRedisClients[config.Name].emplace_back(redisClient);
        return redisClient.get();
	}

    bool RedisComponent::Ping(TcpRedisClient * redisClient)
    {
        std::shared_ptr<RedisRequest> request = RedisRequest::Make("PING");
        std::shared_ptr<RedisResponse> response = this->Run(redisClient, request);
        return response != nullptr && !response->HasError();
    }

    TcpRedisClient * RedisComponent::GetClient(const std::string& name)
    {
        auto iter = this->mRedisClients.find(name);
        if (iter == this->mRedisClients.end() || iter->second.empty())
        {
            return nullptr;
        }
        std::shared_ptr<TcpRedisClient> tempRedisClint = iter->second.front();
        for (std::shared_ptr<TcpRedisClient> redisClient: iter->second)
        {
            if(redisClient->WaitSendCount() <= 5)
            {
                return redisClient.get();
            }
            if(tempRedisClint->WaitSendCount() < redisClient->WaitSendCount())
            {
                tempRedisClint = redisClient;
            }
        }
        return tempRedisClint.get();
    }

    std::shared_ptr<RedisResponse> RedisComponent::Run(
        TcpRedisClient * redisClientContext, std::shared_ptr<RedisRequest> request)
    {
#ifdef __DEBUG__
        ElapsedTimer elapsedTimer;
#endif
		redisClientContext->SendCommand(request);
        std::shared_ptr<RedisResponse> redisResponse = this->AddTask(request->MakeTask())->Await();
#ifdef __DEBUG__
        LOG_INFO(request->GetCommand() << " use time = [" << elapsedTimer.GetMs() << "ms]");
#endif
        if (redisResponse->HasError())
        {
            LOG_ERROR(request->ToJson());
            LOG_ERROR(redisResponse->GetString());
        }
        return redisResponse;
    }

    std::shared_ptr<RedisResponse> RedisComponent::Run(const std::string &name, std::shared_ptr<RedisRequest> request)
    {
        TcpRedisClient * redisClientContext = this->GetClient(name);
        if(redisClientContext == nullptr)
        {
            CONSOLE_LOG_ERROR("not find redis client : " << name);
            return nullptr;
        }
        return this->Run(redisClientContext, request);
    }
}
