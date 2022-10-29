#include"RedisComponent.h"
#include"Lua/LuaRedis.h"
#include"File/DirectoryHelper.h"
#include"Lua/ClassProxyHelper.h"
#include"Client/TcpRedisClient.h"
#include"Component/NetThreadComponent.h"
#include"Component/RedisScriptComponent.h"
#include"Component/RedisStringComponent.h"
namespace Sentry
{
    bool RedisComponent::Awake()
    {
        LOG_CHECK_RET_FALSE(RedisConfig::Inst());
        LOG_CHECK_RET_FALSE(RedisConfig::Inst()->Has("main"));
        LOG_CHECK_RET_FALSE(this->mApp->AddComponent<RedisScriptComponent>());
        LOG_CHECK_RET_FALSE(this->mApp->AddComponent<RedisStringComponent>());
        return true;
    }

    bool RedisComponent::LateAwake()
    {
        std::vector<RedisClientConfig> configs;
        LOG_CHECK_RET_FALSE(RedisConfig::Inst()->Get(configs));
        for(const RedisClientConfig & config : configs)
        {
            for (int index = 0; index < config.Count; index++)
            {
                LOG_CHECK_RET_FALSE(this->MakeRedisClient(config));
            }
        }
        return true;
    }

	void RedisComponent::OnConnectSuccessful(const std::string& address)
	{
		LOG_INFO("redis client [" << address << "] auth successful");
	}

	void RedisComponent::OnMessage(const std::string& address, std::shared_ptr<RedisResponse> message)
	{
		if(message->HasError())
		{
			LOG_ERROR(message->GetString());
		}
		long long id = message->TaskId();
		this->OnResponse(id, message);
	}

    bool RedisComponent::Start()
    {
        std::vector<RedisClientConfig> configs;
        LOG_CHECK_RET_FALSE(RedisConfig::Inst()->Get(configs));
        for(const RedisClientConfig & config : configs)
        {
            if(!this->Ping(this->GetClient(config.Name)))
            {
                CONSOLE_LOG_ERROR("start " << config.Name << " redis client error");
                return false;
            }
        }
        return true;
    }

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
#ifdef __DEBUG__
        ElapsedTimer elapsedTimer;
#endif
		redisClientContext->Send(request);
        std::shared_ptr<RedisTask> redisTask = request->MakeTask();
        std::shared_ptr<RedisResponse> redisResponse = this->AddTask(redisTask->GetRpcId(), redisTask)->Await();
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

	void RedisComponent::OnLuaRegister(Lua::ClassProxyHelper& luaRegister)
	{
		luaRegister.BeginRegister<RedisComponent>();
		luaRegister.PushExtensionFunction("Run", Lua::Redis::Run);
		luaRegister.PushExtensionFunction("Call", Lua::Redis::Call);
        luaRegister.PushExtensionFunction("Write", Lua::Redis::Send);
    }
}

namespace Sentry
{


}
