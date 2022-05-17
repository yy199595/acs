#include"MainRedisComponent.h"

#include"App/App.h"
#include"Util/DirectoryHelper.h"
#include"Script/ClassProxyHelper.h"
#include"Global/ServiceConfig.h"
#include"Component/Scene/NetThreadComponent.h"
#include"DB/Redis/RedisClientContext.h"
#include"Component/Rpc/RpcHandlerComponent.h"
#include"Component/RpcService/LocalServiceComponent.h"
namespace Sentry
{
	AutoRedisLock::AutoRedisLock(MainRedisComponent* component, const std::string& key)
		: mRedisComponent(component), mKey(key)
	{
		this->mIsLock = this->mRedisComponent->Lock(key);
	}
	AutoRedisLock::~AutoRedisLock()
	{
		if(this->mIsLock)
		{
			this->mRedisComponent->UnLock(this->mKey);
		}
	}
}

namespace Sentry
{
	bool MainRedisComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->LoadRedisConfig());
		this->mTaskComponent = this->GetComponent<TaskComponent>();
		this->mTimerComponent = this->GetComponent<TimerComponent>();
		LOG_CHECK_RET_FALSE(this->GetComponent<NetThreadComponent>());
		this->mRpcComponent = this->GetComponent<RpcHandlerComponent>();
		this->GetConfig().GetListenerAddress("rpc", this->mRpcAddress);
		return true;
	}

	bool MainRedisComponent::LoadRedisConfig()
	{
		this->mConfig = this->GetConfig().GetRedisConfig("main");
		return this->mConfig != nullptr;
	}

	bool MainRedisComponent::GetLuaScript(const std::string& file, std::string& command)
	{
		auto iter = this->mLuaCommandMap.find(file);
		if (iter != this->mLuaCommandMap.end())
		{
			command = iter->second;
			return true;
		}
		return false;
	}

	bool MainRedisComponent::OnStart()
	{
		this->mRedisClient = this->MakeRedisClient();
#ifdef __DEBUG__
		this->mDebugClient = this->MakeRedisClient();
#endif
		this->mSubRedisClient = this->MakeRedisClient();
		if (this->mSubRedisClient == nullptr || this->mRedisClient == nullptr)
		{
			LOG_ERROR("connect redis " << this->mConfig->Address << " failure");
			return false;
		}

#ifdef __DEBUG__
		//this->Run("FLUSHALL")->IsOk();
		std::shared_ptr<RedisRequest> request = RedisRequest::Make("FLUSHALL");
		std::shared_ptr<RedisResponse> response = std::make_shared<RedisResponse>();
		if (this->mRedisClient->Run(request, response) != XCode::Successful || !response->IsOk())
		{
			return false;
		}
#endif

		for (const std::string& path: this->mConfig->LuaFiles)
		{
			std::string key;
			if (!this->mRedisClient->LoadLuaScript(path, key))
			{
				return false;
			}
			std::string fileName, director;
			if (!Helper::Directory::GetDirAndFileName(path, director, fileName))
			{
				return false;
			}
			LOG_WARN(fileName << "  " << key);
			this->mLuaCommandMap.emplace(fileName, key);
			LOG_INFO("load redis script " << path << " successful");
		}

		LOG_CHECK_RET_FALSE(this->SubEvent());
#ifdef __DEBUG__
		this->mTaskComponent->Start(&MainRedisComponent::StartDebugRedis, this);
#endif
		this->mTaskComponent->Start(&MainRedisComponent::StartPubSub, this);
		this->mTaskComponent->Start(&MainRedisComponent::CheckRedisClient, this);
		return true;
	}
#ifdef __DEBUG__
	void MainRedisComponent::StartDebugRedis()
	{
		std::shared_ptr<RedisResponse> response(new RedisResponse());
		std::shared_ptr<RedisRequest> request = RedisRequest::Make("MONITOR");
		if(this->mDebugClient->Run(request, response) != XCode::Successful)
		{
			LOG_ERROR("debug redis error");
			return;
		}
		while(this->mDebugClient != nullptr)
		{
			if(!this->mDebugClient->IsOpen())
			{
				int count = 0;
				XCode code = this->mDebugClient->StartConnect();
				if (code == XCode::RedisAuthFailure)
				{
					LOG_FATAL("debug client auth failure");
					return;
				}
				while (code != XCode::Successful)
				{
					LOG_ERROR("debug client connect redis "
							<< this->mConfig->Address << " failure count = " << count++);
					this->mTaskComponent->Sleep(3000);
					code = this->mDebugClient->StartConnect();
				}
				this->SubscribeChannel(this->mRpcAddress);
				LOG_DEBUG("debug redis client connect successful");
			}
			std::shared_ptr<RedisResponse> debugResponse(new RedisResponse());
			if(this->mDebugClient->WaitRedisResponse(debugResponse) == XCode::Successful)
			{
				std::string content;
				if(debugResponse->GetString(content))
				{
					LOG_DEBUG("[redis command] = " << content);
				}
			}
		}
	}
#endif

	bool MainRedisComponent::SubEvent()
	{
		if (!this->SubscribeChannel(this->mRpcAddress))
		{
			return false;
		}
		std::vector<Component*> components;
		this->GetApp()->GetComponents(components);
		for (Component* component: components)
		{
			LocalServiceComponent* localServiceComponent = component->Cast<LocalServiceComponent>();
			if (localServiceComponent != nullptr && localServiceComponent->LoadEvent())
			{
				if (!this->SubscribeChannel(localServiceComponent->GetName()))
				{
					return false;
				}
			}
		}
		return true;
	}

	std::shared_ptr<RedisClientContext> MainRedisComponent::MakeRedisClient()
	{
#ifdef ONLY_MAIN_THREAD
		IAsioThread& workThread = App::Get()->GetTaskScheduler();
#else
		NetThreadComponent * threadPoolComponent = this->GetComponent<NetThreadComponent>();
		IAsioThread& workThread = threadPoolComponent->AllocateNetThread();
#endif
		size_t count = 0;
		const std::string& ip = this->mConfig->Ip;
		unsigned short port = this->mConfig->Port;
		std::shared_ptr<SocketProxy> socketProxy = std::make_shared<SocketProxy>(workThread, ip, port);
		std::shared_ptr<RedisClientContext> redisCommandClient = std::make_shared<RedisClientContext>(socketProxy,
				this->mConfig);

		XCode code = redisCommandClient->StartConnect();
		if (code == XCode::RedisAuthFailure)
		{
			LOG_ERROR("redis auth failure");
			return nullptr;
		}

		while (code != XCode::Successful)
		{
			LOG_ERROR(this->mConfig->Name << " connect redis ["
										  << this->mConfig->Address << "] failure count = " << count++);
			this->mTaskComponent->Sleep(3000);
			code = redisCommandClient->StartConnect();
		}
		LOG_INFO(this->mConfig->Name << " connect redis [" << this->mConfig->Address << "] successful");
		return redisCommandClient;
	}

	long long MainRedisComponent::Publish(const std::string& channel, const std::string& message)
	{
		std::shared_ptr<RedisResponse> response = std::make_shared<RedisResponse>();
		std::shared_ptr<RedisRequest> request = RedisRequest::Make("PUBLISH", channel, message);
		if (this->mRedisClient->Run(request, response) != XCode::Successful)
		{
			LOG_ERROR("redis net work error publish error");
			return -1;
		}
		return response->GetNumber();
	}

	void MainRedisComponent::CheckRedisClient()
	{
		while (this->mSubRedisClient != nullptr)
		{
			this->mTaskComponent->Sleep(5000);
		}
	}

	bool MainRedisComponent::SubscribeChannel(const std::string& channel)
	{
		assert(!channel.empty());
		std::shared_ptr<RedisResponse> response = std::make_shared<RedisResponse>();
		std::shared_ptr<RedisRequest> request = RedisRequest::Make("SUBSCRIBE", channel);
		if (this->mSubRedisClient->Run(request, response) != XCode::Successful)
		{
			LOG_ERROR("redis net work error sub " << channel << " failure");
			return false;
		}
		LOG_INFO("subscribe channel [" << channel << "] successful");
		return true;
	}

	void MainRedisComponent::StartPubSub()
	{
		while (this->mSubRedisClient)
		{
			if (!this->mSubRedisClient->IsOpen())
			{
				int count = 0;
				XCode code = this->mSubRedisClient->StartConnect();
				if (code == XCode::RedisAuthFailure)
				{
					LOG_FATAL("sub client auth failure");
					return;
				}
				while (code != XCode::Successful)
				{
					LOG_ERROR("subclient connect redis "
							<< this->mConfig->Address << " failure count = " << count++);
					this->mTaskComponent->Sleep(3000);
					code = this->mSubRedisClient->StartConnect();
				}
				this->SubscribeChannel(this->mRpcAddress);
				LOG_DEBUG("subscribe redis client connect successful");
			}
			std::shared_ptr<RedisResponse> redisResponse(new RedisResponse());
			XCode code = this->mSubRedisClient->WaitRedisResponse(redisResponse);
			if (code != XCode::Successful || redisResponse->HasError())
			{
				LOG_FATAL("sub redis client error");
				continue;
			}

			std::string type;
			if (redisResponse->GetArraySize() == 3 && redisResponse->GetString(type) && type == "message")
			{
				std::string channel, message;
				redisResponse->GetString(channel, 1);
				redisResponse->GetString(message, 2);
				if(!this->HandlerEvent(channel, message))
				{
					LOG_ERROR("handler " << channel << " error");
				}
			}
		}
	}

	bool MainRedisComponent::HandlerEvent(const std::string& channel, const std::string& message)
	{
		if (message[0] == '+') //请求
		{
			const char* data = message.c_str() + 1;
			const size_t size = message.size() - 1;
			std::shared_ptr<com::Rpc::Request> request(new com::Rpc::Request());
			if (!request->ParseFromArray(data, size))
			{
				LOG_ERROR("parse message error");
				return false;
			}
			this->mRpcComponent->OnRequest(request);
			return true;
		}
		else if (message[0] == '-') //回复
		{
			const char* data = message.c_str() + 1;
			const size_t size = message.size() - 1;
			std::shared_ptr<com::Rpc::Response> response(new com::Rpc::Response());
			if (!response->ParseFromArray(data, size))
			{
				LOG_ERROR("parse message error");
				return false;
			}
			this->mRpcComponent->OnResponse(response);
			return true;
		}
		LocalServiceComponent* localServiceComponent = this->GetComponent<LocalServiceComponent>(channel);
		if (localServiceComponent == nullptr)
		{
			return false;
		}
		std::string eveId;
		std::shared_ptr<Json::Reader> jsonReader(new Json::Reader());
		LOG_CHECK_RET_FALSE(jsonReader->ParseJson(message));
		LOG_CHECK_RET_FALSE(jsonReader->GetMember("eveId", eveId));
		localServiceComponent->Invoke(eveId, jsonReader);
		return true;
	}

	void MainRedisComponent::GetAllAddress(std::vector<std::string>& chanels)
	{
		std::shared_ptr<RedisResponse> response(new RedisResponse());
		std::shared_ptr<RedisRequest> request = RedisRequest::Make("PUBSUB", "CHANNELS", "*:*");
		if(this->mRedisClient->Run(request, response) == XCode::Successful)
		{
			for(size_t index = 0; index < response->GetArraySize(); index++)
			{
				std::string channel;
				if(response->GetString(channel, index))
				{
					chanels.emplace_back(channel);
				}
			}
		}
	}

	long long MainRedisComponent::AddCounter(const string& key)
	{
		std::shared_ptr<RedisResponse> response(new RedisResponse());
		std::shared_ptr<RedisRequest> request = RedisRequest::Make("INCR", key);
		if(this->mRedisClient->Run(request, response) != XCode::Successful)
		{
			return 0;
		}
		return response->GetNumber();
	}

	long long MainRedisComponent::SubCounter(const std::string& key)
	{
		std::shared_ptr<RedisResponse> response(new RedisResponse());
		std::shared_ptr<RedisRequest> request = RedisRequest::Make("DECR", key);
		if(this->mRedisClient->Run(request, response) != XCode::Successful)
		{
			return 0;
		}
		return response->GetNumber();
	}

	bool MainRedisComponent::Lock(const string& key, int timeout)
	{
		Json::Writer jsonWriter;
		jsonWriter.AddMember("key", key);
		jsonWriter.AddMember("time", timeout);
		if(!this->CallLua("lock.lock", jsonWriter))
		{
			return false;
		}
		LOG_DEBUG("redis lock " << key << " get successful");
		this->mLockTimers[key] = this->mTimerComponent->DelayCall(
				(float)timeout - 0.5f, &MainRedisComponent::OnLockTimeout, this, key, timeout);
		return true;
	}

	bool MainRedisComponent::UnLock(const string& key)
	{
		std::shared_ptr<RedisResponse> response1(new RedisResponse());
		std::shared_ptr<RedisRequest> request1 = RedisRequest::Make( "DEL", key);
		if (this->mRedisClient->Run(request1, response1) != XCode::Successful)
		{
			LOG_ERROR("unlock " << key << " failure");
			return false;
		}

		auto iter = this->mLockTimers.find(key);
		if(iter != this->mLockTimers.end())
		{
			unsigned int id = iter->second;
			this->mLockTimers.erase(iter);
			this->mTimerComponent->CancelTimer(id);
		}
		LOG_INFO(key << " unlock successful");
		return true;
	}

	void MainRedisComponent::OnLockTimeout(const std::string& key, int timeout)
	{
		auto iter = this->mLockTimers.find(key);
		if (iter != this->mLockTimers.end())
		{
			this->mLockTimers.erase(iter);
			this->mTaskComponent->Start([this, key, timeout]()
			{
				std::shared_ptr<RedisResponse> response(new RedisResponse());
				std::shared_ptr<RedisRequest> request = RedisRequest::Make("SETEX", key, 10, 1);
				if(this->mRedisClient->Run(request, response) == XCode::Successful && response->IsOk())
				{
					this->mLockTimers[key] = this->mTimerComponent->DelayCall(
							(float)timeout - 0.5f, &MainRedisComponent::OnLockTimeout,this, key, timeout);
					return;
				}
				LOG_ERROR("redis lock " << key << " delay failure");
			});
		}
	}

	bool MainRedisComponent::CallLua(const std::string& fullName, Json::Writer& json)
	{
		std::shared_ptr<Json::Reader> response(new Json::Reader());
		return this->CallLua(fullName, json, response);
	}

	bool MainRedisComponent::CallLua(const std::string& fullName, Json::Writer& json, std::shared_ptr<Json::Reader> response)
	{
		std::string tag;
		size_t pos = fullName.find('.');
		assert(pos != std::string::npos);
		std::string tab = fullName.substr(0, pos);
		std::string func = fullName.substr(pos + 1);
		if (!this->GetLuaScript(fmt::format("{0}.lua", tab), tag))
		{
			LOG_ERROR("not find redis script lock.lua");
			return false;
		}
		std::shared_ptr<RedisResponse> response1(new RedisResponse());
		std::shared_ptr<RedisRequest> request = RedisRequest::MakeLua(tag, func, json);
#ifdef __DEBUG__
		LOG_INFO(fullName << " json = " << json.ToJsonString());
#endif
		if (this->mRedisClient->Run(request, response1) != XCode::Successful)
		{
			return false;
		}
		if(!response1->GetString(this->mResString) || !response->ParseJson(this->mResString))
		{
			return false;
		}
		bool res = false;
		return response->GetMember("res", res) && res;
	}
}
