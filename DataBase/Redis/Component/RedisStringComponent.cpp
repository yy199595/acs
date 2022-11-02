//
// Created by zmhy0073 on 2022/11/2.
//

#include"RedisStringComponent.h"
#include"RedisComponent.h"
namespace Sentry
{
    bool RedisStringComponent::LateAwake()
    {
        this->mComponent = this->GetComponent<RedisComponent>();
        return true;
    }
    long long RedisStringComponent::AddCounter(const std::string &key)
    {
        TcpRedisClient *redisClientContext = this->mComponent->GetClient("main");
        std::shared_ptr<RedisResponse> redisResponse = this->mComponent->Run(redisClientContext, "INCR", key);
        return (redisResponse != nullptr && !redisResponse->HasError()) ? redisResponse->GetNumber() : -1;
    }

    long long RedisStringComponent::SubCounter(const std::string &key)
    {
        TcpRedisClient * redisClient = this->mComponent->GetClient("main");
        std::shared_ptr<RedisResponse> redisResponse = this->mComponent->Run(redisClient, "DECR", key);
        return (redisResponse != nullptr && !redisResponse->HasError()) ? redisResponse->GetNumber() : -1;
    }

    bool RedisStringComponent::Set(const std::string &db, const std::string &key, const std::string &value)
    {
        TcpRedisClient *redisClient = this->mComponent->GetClient(db);
        std::shared_ptr<RedisResponse> redisResponse = this->mComponent->Run(redisClient, "SET", key, value);
        return redisResponse != nullptr && !redisResponse->HasError();
    }

    std::unique_ptr<std::string> RedisStringComponent::Get(const std::string &db, const std::string &key)
    {
        TcpRedisClient *redisClient = this->mComponent->GetClient(db);
        std::shared_ptr<RedisResponse> redisResponse = this->mComponent->Run(redisClient, "GET", key);
        if(redisResponse == nullptr || redisResponse->HasError())
        {
            return nullptr;
        }
        const std::string & str = redisResponse->GetString();
        return std::make_unique<std::string>(std::move(str));
    }
}