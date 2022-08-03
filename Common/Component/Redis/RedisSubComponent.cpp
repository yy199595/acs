//
// Created by zmhy0073 on 2022/8/3.
//

#include "RedisSubComponent.h"
#include"Component/Scene/NetEventComponent.h"
namespace Sentry
{
    bool RedisSubComponent::LateAwake()
    {
        LOG_CHECK_RET_FALSE(RedisComponent::LateAwake());
        const ServerConfig & config = this->GetApp()->GetConfig();
        const rapidjson::Value * jsonValue = config.GetJsonValue("redis", "sub");
        if(jsonValue == nullptr)
        {
            LOG_ERROR("not find main redis config");
            return false;
        }
        LOG_CHECK_RET_FALSE(this->ParseConfig("sub", *jsonValue));
        return true;
    }

    bool RedisSubComponent::StartSubChannel()
    {
        if (!this->SubscribeChannel(this->mAddress))
        {
            return false;
        }
        std::vector<Component*> components;
        this->GetApp()->GetComponents(components);
        for (Component* component: components)
        {
            NetEventComponent* localServiceComponent = component->Cast<NetEventComponent>();
            if (localServiceComponent != nullptr)
            {
                if(!localServiceComponent->StartRegisterEvent())
                {
                    LOG_INFO(component->GetName() << " start listen event failure");
                    return false;
                }
                LOG_INFO(component->GetName() << " start listen event successful");
            }
        }
        this->mSubClient->StartReceiveMessage();
        return true;
    }


    void RedisSubComponent::OnNotFindResponse(long long taskId, std::shared_ptr<RedisResponse> response)
    {
        if(response->GetType() == RedisRespType::REDIS_ARRAY && response->GetArraySize() == 3)
        {
            const RedisAny * redisAny1 = response->Get(0);
            const RedisAny * redisAny2 = response->Get(1);
            const RedisAny * redisAny3 = response->Get(2);
            if(redisAny1->IsString() && redisAny2->IsString() && redisAny3->IsString())
            {
                if(static_cast<const RedisString*>(redisAny1)->GetValue() == "message")
                {
                    const std::string & channel = redisAny2->Cast<RedisString>()->GetValue();
                    const std::string & message = redisAny3->Cast<RedisString>()->GetValue();
                    // 处理事件
                }
            }
        }
    }

    long long RedisSubComponent::Publish(const std::string& channel, const std::string& message)
    {
        std::shared_ptr<RedisRequest> request = RedisRequest::Make("PUBLISH", channel, message);
        std::shared_ptr<RedisResponse> redisResponse = this->Run(this->mSubClient, request);
        return redisResponse == nullptr ? 0 : redisResponse->GetNumber();
    }

    bool RedisSubComponent::SubscribeChannel(const std::string& channel)
    {
        std::shared_ptr<RedisRequest> request = RedisRequest::Make("SUBSCRIBE", channel);
        std::shared_ptr<RedisResponse> redisResponse = this->Run(this->mSubClient, request);
        if(redisResponse->GetArraySize() == 3 && redisResponse->Get(2)->IsNumber())
        {
            if(((const RedisNumber*)redisResponse->Get(2))->GetValue() > 0)
            {
                LOG_INFO("sub " << channel << " successful");
                return true;
            }
        }
        LOG_INFO("sub " << channel << " failure");
        return false;
    }
}