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
        const char * cmd = RedisCommand::Str::AddOne;
        std::shared_ptr<RedisResponse> redisResponse = this->mComponent->Run(cmd, key);
        return (redisResponse != nullptr && !redisResponse->HasError()) ? redisResponse->GetNumber() : -1;
    }

    long long RedisStringComponent::SubCounter(const std::string &key)
    {
        const char * cmd = RedisCommand::Str::SubOne;
        std::shared_ptr<RedisResponse> redisResponse = this->mComponent->Run(cmd, key);
        return (redisResponse != nullptr && !redisResponse->HasError()) ? redisResponse->GetNumber() : -1;
    }

    bool RedisStringComponent::Set(const std::string &key, const std::string &value)
    {
        const char * cmd = RedisCommand::Str::Set;
        std::shared_ptr<RedisResponse> redisResponse = this->mComponent->Run(cmd, key, value);
        return redisResponse != nullptr && !redisResponse->HasError();
    }

    std::unique_ptr<std::string> RedisStringComponent::Get(const std::string &key)
    {
        const char * cmd = RedisCommand::Str::Get;
        std::shared_ptr<RedisResponse> redisResponse = this->mComponent->Run(cmd, key);
        if(redisResponse == nullptr || redisResponse->HasError())
        {
            return nullptr;
        }
        const std::string & str = redisResponse->GetString();
        return std::make_unique<std::string>(std::move(str));
    }

    std::unique_ptr<std::string> RedisStringComponent::Append(const std::string &key, const std::string &value)
    {
        const char * cmd = RedisCommand::Str::Append;
        std::shared_ptr<RedisResponse> redisResponse = this->mComponent->Run(cmd, key, value);
        if(redisResponse == nullptr || redisResponse->HasError())
        {
            return nullptr;
        }
        const std::string & str = redisResponse->GetString();
        return std::make_unique<std::string>(std::move(str));
    }
}