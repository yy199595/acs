#include"RedisComponent.h"
#include"Redis/Lua/LuaRedis.h"

#include"Lua/Engine/ModuleClass.h"
#include"Redis/Client/RedisClient.h"
#include"Server/Component/ThreadComponent.h"
#include"Redis/Component/RedisLuaComponent.h"
#include"Redis/Component/RedisStringComponent.h"
#include"Rpc/Component/DispatchComponent.h"
#include"Entity/Actor/App.h"
#include"Redis/Config/RedisConfig.h"
#ifdef __DEBUG__
#include "Timer/Timer/ElapsedTimer.h"
#endif
namespace Tendo
{
    bool RedisComponent::Awake()
    {
		std::string path;
		this->mIndex = 0;
		const ServerConfig * config = ServerConfig::Inst();
		LOG_CHECK_RET_FALSE(config->GetPath("db", path))
		LOG_CHECK_RET_FALSE(this->mConfig.LoadConfig(path))
        LOG_CHECK_RET_FALSE(this->mApp->AddComponent<RedisLuaComponent>());
        LOG_CHECK_RET_FALSE(this->mApp->AddComponent<RedisStringComponent>());

		Asio::Context& io = this->mApp->MainThread();
		std::shared_ptr<Tcp::SocketProxy> socket = std::make_shared<Tcp::SocketProxy>(io);

		socket->Init(this->Config().Address.Ip, this->Config().Address.Port);
		this->mMainClient =	std::make_shared<RedisClient>(socket, this->mConfig.Config(), this);
		return this->mMainClient->AuthUser();	
    }

    bool RedisComponent::LateAwake()
    {
		for (int index = 0; index < this->Config().Count; index++)
		{
			this->MakeRedisClient(this->Config());
		}
		this->mDispatchComponent = this->GetComponent<DispatchComponent>();
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
		if(message->TaskId() != 0)
		{
			int id = message->TaskId();
			this->OnResponse(id, message);
		}
		else if(message->GetType() == RedisRespType::REDIS_ARRAY
			&& message->GetArraySize() == 3)
		{
			const RedisString * redisAny2 = message->Get(1)->Cast<RedisString>();
			const RedisString * redisAny3 = message->Get(2)->Cast<RedisString>();
			std::shared_ptr<Msg::Packet> request = std::make_shared<Msg::Packet>();
			{
				request->SetType(Msg::Type::SubPublish);
				request->SetContent(redisAny3->GetValue());
				request->GetHead().Add("channel", redisAny2->GetValue());
			}
			this->mDispatchComponent->OnMessage(request);
		}
	}

    RedisClient * RedisComponent::MakeRedisClient(const RedisClientConfig & config)
    {
        const std::string & ip = config.Address.Ip;
        const unsigned int port = config.Address.Port;
        ThreadComponent * component = this->GetComponent<ThreadComponent>();
        std::shared_ptr<Tcp::SocketProxy> socketProxy = component->CreateSocket(ip, port);
        if(socketProxy == nullptr)
        {
            return nullptr;
        }
        std::shared_ptr<RedisClient> redisClient =
            std::make_shared<RedisClient>(socketProxy, config, this);
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

    RedisClient * RedisComponent::GetClient(size_t index)
    {
		if (this->mRedisClients.empty())
		{
			return nullptr;
		}
		if (this->mIndex >= this->mRedisClients.size())
		{
			this->mIndex = 0;
		}
		return this->mRedisClients[this->mIndex++].get();		
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
        LOG_INFO("async redis cmd (" << request->GetCommand() << ") use time = [" << elapsedTimer.GetMs() << "ms]");
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
#ifdef __DEBUG__
		ElapsedTimer elapsedTimer;
#endif
		std::shared_ptr<RedisResponse> response = 
			this->mMainClient->SyncCommand(request);
#ifdef __DEBUG__
		LOG_INFO("sync redis cmd (" << request->GetCommand() << ") use time = [" << elapsedTimer.GetMs() << "ms]");
#endif
		if (response != nullptr && response->HasError())
		{
			LOG_ERROR(request->ToJson());
			LOG_ERROR(response->GetString());
		}
		return response;
	}

	void RedisComponent::OnLuaRegister(Lua::ModuleClass &luaRegister, std::string &name)
	{
		name = "Redis";
		luaRegister.AddFunction("Run", Lua::Redis::Run);
		luaRegister.AddFunction("Call", Lua::Redis::Call);
        luaRegister.AddFunction("Send", Lua::Redis::Send);
		luaRegister.AddFunction("Sync", Lua::Redis::SyncRun);
    }

	bool RedisComponent::Send(const std::shared_ptr<RedisRequest>& request)
	{
		RedisClient * redisClientContext = this->GetClient();
		if(redisClientContext == nullptr)
		{
			return false;
		}
		redisClientContext->Send(request);
		return true;
	}
	bool RedisComponent::Send(const std::shared_ptr<RedisRequest>& request, int& id)
	{
		RedisClient * redisClientContext = this->GetClient();
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
			redisClient->Send(RedisRequest::Make("QUIT"));
		}
	}
}
