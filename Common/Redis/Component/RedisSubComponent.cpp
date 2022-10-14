//
// Created by zmhy0073 on 2022/8/3.
//

#include"RedisSubComponent.h"
#include"Lua/Function.h"
#include"Json/Lua/Json.h"
#include"Config/ServerConfig.h"
#include"Component/LuaScriptComponent.h"
#include"Component/TextConfigComponent.h"
#include"Component/RedisChannelComponent.h"
namespace Sentry
{
    bool RedisSubComponent::OnInitRedisClient(RedisConfig config)
    {
        TextConfigComponent * textConfigComponent = this->GetComponent<TextConfigComponent>();
        const ServerConfig * localServerConfig = textConfigComponent->GetTextConfig<ServerConfig>();
        if(!config.Channels.empty())
        {
            config.LuaFiles.clear();
            TcpRedisClient * redisClient = this->MakeRedisClient(config);
            localServerConfig->GetLocation("rpc", this->mLocalHost);
            if(redisClient == nullptr || !this->Ping(redisClient))
            {
                return false;
            }
        }
        return true;
    }

    void RedisSubComponent::OnNotFindResponse(long long taskId, std::shared_ptr<RedisResponse> response)
    {
        if (response->GetType() != RedisRespType::REDIS_ARRAY || response->GetArraySize() != 3)
        {
            return;
        }
        const RedisAny *redisAny1 = response->Get(0);
        const RedisAny *redisAny2 = response->Get(1);
        const RedisAny *redisAny3 = response->Get(2);
        if (redisAny1->IsString() && redisAny2->IsString() && redisAny3->IsString())
        {
            if (static_cast<const RedisString *>(redisAny1)->GetValue() == "message")
            {
                const std::string &channel = redisAny2->Cast<RedisString>()->GetValue();
                const std::string &message = redisAny3->Cast<RedisString>()->GetValue();
                if (channel == this->mLocalHost && !this->Invoke(message))
                {
                    LOG_ERROR("handler redis sub event error channel = "
                                  << channel << " message = " << message);
                    return;
                }
                if (!this->Invoke(channel, message))
                {
                    LOG_ERROR("handler redis sub event error channel = "
                                  << channel << " message = " << message);
                }
            }
        }
    }

    bool RedisSubComponent::Invoke(const std::string &message)
    {
        return false;
    }

    bool RedisSubComponent::Invoke(const std::string &channel, const std::string &message)
    {
        std::vector<std::string> methodInfos;
        google::protobuf::SplitStringUsing(channel, ".", &methodInfos);
        if (methodInfos.size() != 2)
        {
            return false;
        }
        const std::string & method = methodInfos[1];
        const std::string & component = methodInfos[0];
        if(this->mLuaComponent != nullptr)
        {
            lua_State * lua = this->mLuaComponent->GetLuaEnv();
            if(Lua::Function::Get(lua, component.c_str(), method.c_str()))
            {
                Lua::Json::Write(lua, message);
                if(lua_pcall(lua, 1, 0, 0) != LUA_OK)
                {
                    LOG_ERROR("handler lua event " << channel << " error = " << lua_tostring(lua, -1));
                    return false;
                }
                return true;
            }
        }

        RedisChannelComponent* localServiceComponent = this->GetComponent<RedisChannelComponent>(component);
        if(localServiceComponent != nullptr)
        {
            std::shared_ptr<Json::Reader> jsonReader(new Json::Reader());
            if((!jsonReader->ParseJson(message)) || (!localServiceComponent->Invoke(method, jsonReader)))
            {
                return false;
            }
            return true;
        }
        return false;
    }

    long long RedisSubComponent::Publish(const std::string & name, const std::string& channel, const std::string& message)
    {
        TcpRedisClient * redisClient = this->GetClient(name);
        if(redisClient == nullptr)
        {
            LOG_FATAL("not find redis client : " << name);
            return 0;
        }
        std::shared_ptr<RedisRequest> request = RedisRequest::Make("PUBLISH", channel, message);
        std::shared_ptr<RedisResponse> redisResponse = this->Run(redisClient, request);
        return redisResponse == nullptr ? 0 : redisResponse->GetNumber();
    }

    bool RedisSubComponent::SubscribeChannel(const std::string & name, const std::string& channel)
    {
        TcpRedisClient * redisClient = this->GetClient(name);
        if(redisClient == nullptr)
        {
            LOG_FATAL("not find redis client : " << name);
            return 0;
        }
        std::shared_ptr<RedisRequest> request = RedisRequest::Make("SUBSCRIBE", channel);
        std::shared_ptr<RedisResponse> redisResponse = this->Run(redisClient, request);
        if(redisResponse == nullptr)
        {
            return false;
        }
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