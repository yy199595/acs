//
// Created by mac on 2022/5/18.
//

#include"RedisComponent.h"
#include"Timer/ElapsedTimer.h"
#include"Component/NetThreadComponent.h"

namespace Sentry
{
    TcpRedisClient * RedisComponent::MakeRedisClient(const RedisClientConfig & config)
	{
        NetThreadComponent * component = this->GetComponent<NetThreadComponent>();
        std::shared_ptr<SocketProxy> socketProxy = component->CreateSocket();
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
        if(!this->IsRunCommand())
        {
            LOG_ERROR(this->GetName() << " not run command");
            return nullptr;
        }
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
