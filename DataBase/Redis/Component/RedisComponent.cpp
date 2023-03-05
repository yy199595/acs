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
        LOG_CHECK_RET_FALSE(this->mApp->AddComponent<RedisScriptComponent>());
        LOG_CHECK_RET_FALSE(this->mApp->AddComponent<RedisStringComponent>());
        return true;
    }

    bool RedisComponent::LateAwake()
    {
		const RedisClientConfig & config
			= RedisConfig::Inst()->Config();
        return this->MakeRedisClient(config);
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
		if (!this->Ping(this->GetClient()))
		{
			CONSOLE_LOG_ERROR("start  redis client error");
			return false;
		}
		return true;
	}

    TcpRedisClient * RedisComponent::MakeRedisClient(const RedisClientConfig & config)
    {
        const std::string & ip = config.Address[0].Ip;
        const unsigned int port = config.Address[0].Port;
        NetThreadComponent * component = this->GetComponent<NetThreadComponent>();
        std::shared_ptr<SocketProxy> socketProxy = component->CreateSocket();
        if(socketProxy == nullptr)
        {
            return nullptr;
        }
        socketProxy->Init(ip, port);
        std::shared_ptr<TcpRedisClient> redisClient =
            std::make_shared<TcpRedisClient>(socketProxy, config, this);
        this->mRedisClients.emplace_back(redisClient);
        return redisClient.get();
    }

    bool RedisComponent::Ping(TcpRedisClient * redisClient)
    {
        std::shared_ptr<RedisRequest> request = RedisRequest::Make("PING");
        std::shared_ptr<RedisResponse> response = this->Run(redisClient, request);
        return response != nullptr && !response->HasError();
    }

    TcpRedisClient * RedisComponent::GetClient()
    {
        std::shared_ptr<TcpRedisClient> tempRedisClint = this->mRedisClients.front();
        for (std::shared_ptr<TcpRedisClient> & redisClient: this->mRedisClients)
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
		std::shared_ptr<RedisResponse> redisResponse =
			this->AddTask(redisTask->GetRpcId(), redisTask)->Await();
#ifdef __DEBUG__
        LOG_INFO(request->GetCommand() << " use time = [" << elapsedTimer.GetMs() << "ms]");
#endif
        if (redisResponse != nullptr && redisResponse->HasError())
        {
            LOG_ERROR(request->ToJson());
            LOG_ERROR(redisResponse->GetString());
        }
        return redisResponse;
    }

    std::shared_ptr<RedisResponse> RedisComponent::Run(std::shared_ptr<RedisRequest> request)
    {
        TcpRedisClient * redisClientContext = this->GetClient();
        if(redisClientContext == nullptr)
        {
            return nullptr;
        }
        return this->Run(redisClientContext, request);
    }

	void RedisComponent::OnLuaRegister(Lua::ClassProxyHelper& luaRegister)
	{
		luaRegister.BeginNewTable("Redis");
		luaRegister.PushExtensionFunction("Run", Lua::Redis::Run);
		luaRegister.PushExtensionFunction("Call", Lua::Redis::Call);
        luaRegister.PushExtensionFunction("Send", Lua::Redis::Send);
    }
}
