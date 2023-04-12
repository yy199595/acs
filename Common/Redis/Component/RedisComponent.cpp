#include"RedisComponent.h"
#include"Redis/Lua/LuaRedis.h"

#include"Lua/Engine/ClassProxyHelper.h"
#include"Redis/Client/TcpRedisClient.h"
#include"Server/Component/ThreadComponent.h"
#include"Redis/Component/RedisScriptComponent.h"
#include"Redis/Component/RedisStringComponent.h"
#include"Entity/Unit/App.h"
#include"Redis/Config/RedisConfig.h"
#ifdef __DEBUG__
#include "Timer/Timer/ElapsedTimer.h"
#endif
namespace Tendo
{
    bool RedisComponent::Awake()
    {
		std::string path;
		const ServerConfig * config = ServerConfig::Inst();
		LOG_CHECK_RET_FALSE(config->GetPath("db", path));
		LOG_CHECK_RET_FALSE(this->mConfig.LoadConfig(path));
        LOG_CHECK_RET_FALSE(this->mApp->AddComponent<RedisScriptComponent>());
        LOG_CHECK_RET_FALSE(this->mApp->AddComponent<RedisStringComponent>());

		Asio::Context& io = this->mApp->MainThread();
		std::shared_ptr<Tcp::SocketProxy> socket = std::make_shared<Tcp::SocketProxy>(io);

		socket->Init(this->Config().Address[0].Ip, this->Config().Address[0].Port);
		this->mMainClient =	std::make_shared<TcpRedisClient>(socket, this->mConfig.Config(), this);
		return this->mMainClient->AuthUser();	
    }

    bool RedisComponent::LateAwake()
    {
		for (int index = 0; index < this->Config().Count; index++)
		{
			this->MakeRedisClient(this->Config());
		}
		return true;
    }

	void RedisComponent::OnConnectSuccessful(const std::string& address)
	{
		LOG_INFO("redis client [" << address << "] auth successful");
	}

	void RedisComponent::OnMessage(std::shared_ptr<RedisResponse> message)
	{
		if(message->HasError())
		{
			LOG_ERROR(message->GetString());
		}
		int id = message->TaskId();
		this->OnResponse(id, message);
	}

    TcpRedisClient * RedisComponent::MakeRedisClient(const RedisClientConfig & config)
    {
        const std::string & ip = config.Address[0].Ip;
        const unsigned int port = config.Address[0].Port;
        ThreadComponent * component = this->GetComponent<ThreadComponent>();
        std::shared_ptr<Tcp::SocketProxy> socketProxy = component->CreateSocket();
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

    bool RedisComponent::Ping(size_t index)
    {
        std::shared_ptr<RedisRequest> request =
			RedisRequest::Make("PING");
        std::shared_ptr<RedisResponse> response = this->Run(request);
        return response != nullptr && !response->HasError();
    }

    TcpRedisClient * RedisComponent::GetClient(size_t index)
    {
		if(this->mRedisClients.empty())
		{
			return nullptr;
		}
		if(index > 0)
		{
			index = index % this->mRedisClients.size();
			return this->mRedisClients[index].get();
		}
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

    std::shared_ptr<RedisResponse> RedisComponent::Run(const std::shared_ptr<RedisRequest>& request)
    {
#ifdef __DEBUG__
        ElapsedTimer elapsedTimer;
#endif
		int taskId = 0;
		if(!this->Send(request, taskId))
		{
			LOG_ERROR("send redis cmd error : "  << request->ToJson());
			return nullptr;
		}
		std::shared_ptr<RedisTask> redisTask = std::make_shared<RedisTask>(taskId);
		std::shared_ptr<RedisResponse> redisResponse = this->AddTask(taskId, redisTask)->Await();
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

	std::shared_ptr<RedisResponse> RedisComponent::SyncRun(const std::shared_ptr<RedisRequest>& request)
	{
		std::shared_ptr<RedisResponse> response = 
			this->mMainClient->SyncCommand(request);
		if (response != nullptr && response->HasError())
		{
			//LOG_ERROR(request->ToJson());
			LOG_ERROR(response->GetString());
		}
		return response;
	}

	void RedisComponent::OnLuaRegister(Lua::ClassProxyHelper& luaRegister)
	{
		luaRegister.BeginNewTable("Redis");
		luaRegister.PushExtensionFunction("Run", Lua::Redis::Run);
		luaRegister.PushExtensionFunction("Call", Lua::Redis::Call);
        luaRegister.PushExtensionFunction("Send", Lua::Redis::Send);
		luaRegister.PushExtensionFunction("SyncRun", Lua::Redis::SyncRun);
    }
	bool RedisComponent::Send(const std::shared_ptr<RedisRequest>& request)
	{
		TcpRedisClient * redisClientContext = this->GetClient();
		if(redisClientContext == nullptr)
		{
			return false;
		}
		redisClientContext->Send(request);
		return true;
	}
	bool RedisComponent::Send(const std::shared_ptr<RedisRequest>& request, int& id)
	{
		TcpRedisClient * redisClientContext = this->GetClient();
		if(redisClientContext == nullptr)
		{
			return false;
		}
		id = this->PopTaskId();
		request->SetRpcTaskId(id);
		redisClientContext->Send(request);
		return true;
	}
	void RedisComponent::OnDestroy()
	{
		this->SyncRun("QUIT");
		for(auto & redisClient : this->mRedisClients)
		{
			std::shared_ptr<RedisRequest> request
					= std::make_shared<RedisRequest>("QUIT");

			redisClient->Send(request);
		}
	}
}
