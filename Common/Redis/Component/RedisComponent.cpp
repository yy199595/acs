#include"RedisComponent.h"
#include"Redis/Lua/LuaRedis.h"

#include"Lua/Engine/ClassProxyHelper.h"
#include"Redis/Client/RedisTcpClient.h"
#include"Server/Component/ThreadComponent.h"
#include"Redis/Component/RedisLuaComponent.h"
#include"Redis/Component/RedisStringComponent.h"
#include"Rpc/Component/DispatchComponent.h"
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
        LOG_CHECK_RET_FALSE(this->mApp->AddComponent<RedisLuaComponent>());
        LOG_CHECK_RET_FALSE(this->mApp->AddComponent<RedisStringComponent>());

		Asio::Context& io = this->mApp->MainThread();
		std::shared_ptr<Tcp::SocketProxy> socket = std::make_shared<Tcp::SocketProxy>(io, "tcp");

		socket->Init(this->Config().Address[0].Ip, this->Config().Address[0].Port);
		this->mMainClient =	std::make_shared<RedisTcpClient>(socket, this->mConfig.Config(), this);
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
				request->SetFrom(this->mSubClient->GetAddress());
				request->GetHead().Add("channel", redisAny2->GetValue());
			}
			this->mDispatchComponent->OnMessage(request);
		}
	}

    RedisTcpClient * RedisComponent::MakeRedisClient(const RedisClientConfig & config)
    {
        const std::string & ip = config.Address[0].Ip;
        const unsigned int port = config.Address[0].Port;
        ThreadComponent * component = this->GetComponent<ThreadComponent>();
        std::shared_ptr<Tcp::SocketProxy> socketProxy = component->CreateSocket("tcp");
        if(socketProxy == nullptr)
        {
            return nullptr;
        }
        socketProxy->Init(ip, port);
        std::shared_ptr<RedisTcpClient> redisClient =
            std::make_shared<RedisTcpClient>(socketProxy, config, this);
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

    RedisTcpClient * RedisComponent::GetClient(size_t index)
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
        std::shared_ptr<RedisTcpClient> tempRedisClint = this->mRedisClients.front();
        for (std::shared_ptr<RedisTcpClient> & redisClient: this->mRedisClients)
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
		RedisTcpClient * redisClientContext = this->GetClient();
		if(redisClientContext == nullptr)
		{
			return false;
		}
		redisClientContext->Send(request);
		return true;
	}
	bool RedisComponent::Send(const std::shared_ptr<RedisRequest>& request, int& id)
	{
		RedisTcpClient * redisClientContext = this->GetClient();
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

	bool RedisComponent::SubChanel(const string& chanel)
	{
		if(this->mSubClient == nullptr)
		{
			const std::string & ip = this->mConfig.Config().Address[0].Ip;
			const unsigned int port = this->mConfig.Config().Address[0].Port;
			ThreadComponent * component = this->GetComponent<ThreadComponent>();
			std::shared_ptr<Tcp::SocketProxy> socketProxy = component->CreateSocket("redis", ip, port);
			this->mSubClient = std::make_shared<RedisTcpClient>(socketProxy, this->mConfig.Config(), this);
		}
		if(this->mSubClient == nullptr)
		{
			return false;
		}
		int taskId = 0;
		std::shared_ptr<RedisRequest> request = RedisRequest::Make("SUBSCRIBE", chanel);
		if(!this->Send(request, taskId))
		{
			LOG_ERROR("send redis cmd error : "  << request->ToJson());
			return false;
		}
		std::shared_ptr<RedisTask> redisTask = std::make_shared<RedisTask>(taskId);
		std::shared_ptr<RedisResponse> redisResponse = this->AddTask(taskId, redisTask)->Await();
		if (redisResponse != nullptr && redisResponse->HasError())
		{
			LOG_ERROR(request->ToJson());
			LOG_ERROR(redisResponse->GetString());
		}
		return redisResponse != nullptr && redisResponse->GetArraySize() == 3;
	}
}
