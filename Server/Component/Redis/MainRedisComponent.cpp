#include"MainRedisComponent.h"

#include"App/App.h"
#include"Util/FileHelper.h"
#include"Util/DirectoryHelper.h"
#include"Script/ClassProxyHelper.h"
#include"Component/Service/SubService.h"
#include"Global/RpcConfig.h"
#include"Component/Scene/ThreadPoolComponent.h"
#include"DB/Redis/RedisClient.h"
namespace Sentry
{
	bool MainRedisComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->LoadRedisConfig());
		this->mTaskComponent = this->GetComponent<TaskComponent>();
		this->mTimerComponent = this->GetComponent<TimerComponent>();
		LOG_CHECK_RET_FALSE(this->GetComponent<ThreadPoolComponent>());
		return true;
	}
	bool MainRedisComponent::LoadRedisConfig()
	{
		this->mConfig.mCount = 3;
		this->GetConfig().GetMember("redis.main", "count", this->mConfig.mCount);
		this->GetConfig().GetMember("redis.main", "lua", this->mConfig.mLuaFiles);
		this->GetConfig().GetMember("redis.main", "passwd", this->mConfig.mPassword);
		LOG_CHECK_RET_FALSE(this->GetConfig().GetMember("redis.main", "ip", this->mConfig.mIp));
		LOG_CHECK_RET_FALSE(this->GetConfig().GetMember("redis.main", "port", this->mConfig.mPort));
		this->mConfig.mAddress = fmt::format("{0}:{1}", this->mConfig.mIp, this->mConfig.mPort);
		return this->GetConfig().GetListenerAddress("rpc", this->mRpcAddress);
	}

	std::shared_ptr<RedisResponse> MainRedisComponent::Call(const std::string& func, std::vector<std::string>& args)
	{
		std::string script;
		const size_t pos = func.find('.');
		if (pos == std::string::npos)
		{
			return nullptr;
		}
		std::string tab = func.substr(0, pos);
		std::string file = fmt::format("{0}.lua", tab);
		if (!this->GetLuaScript(file, script))
		{
			LOG_ERROR("not find redis script " << file);
			return nullptr;
		}
		std::shared_ptr<RedisRequest> redisCmdRequest(new RedisRequest("EVALSHA"));
		redisCmdRequest->InitParameter(script, (int)args.size() + 1, func.substr(pos + 1));
		for (const std::string& val : args)
		{
			redisCmdRequest->AddParameter(val);
		}
		return this->InvokeCommand(redisCmdRequest);
	}

	std::shared_ptr<RedisResponse> MainRedisComponent::InvokeCommand(std::shared_ptr<RedisRequest> request)
	{
		std::shared_ptr<RedisClient> redisClient = this->AllotRedisClient();
#ifdef __DEBUG__
		//LOG_WARN("redis command name = " << request->ToJson());
#endif
		if (redisClient == nullptr)
		{
			LOG_ERROR("allot redis client failure");
			return nullptr;
		}
		std::shared_ptr<RedisResponse> response = redisClient->InvokeCommand(request)->Await();
		if(response->HasError())
		{
			LOG_ERROR(request->ToJson());
			LOG_ERROR(response->GetValue());
		}
//#ifdef __DEBUG__
//		LOG_INFO("invoke redis command use time [" << elapsedTimer.GetMs() << "ms]");
//#endif
		if (!this->mWaitAllotClients.empty())
		{
			auto taskClient = this->mWaitAllotClients.front();
			this->mWaitAllotClients.pop();
			taskClient->SetResult(redisClient);
			return response;
		}
		this->mFreeClients.emplace(redisClient);
		return response;
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
		this->mSubRedisClient = this->MakeRedisClient();
		if(this->mSubRedisClient == nullptr)
		{
			LOG_ERROR("connect redis " << this->mConfig.mAddress << " failure");
			return false;
		}

		for (size_t index = 0; index < this->mConfig.mCount; index++)
		{
			std::shared_ptr<RedisClient> redisClient = this->MakeRedisClient();
			if (redisClient == nullptr)
			{
				LOG_FATAL("connect redis " << this->mConfig.mAddress << " failure");
				return false;
			}
			this->mFreeClients.push(redisClient);
		}
		std::shared_ptr<RedisClient> redisClient = this->mFreeClients.front();
		for (const std::string& path : this->mConfig.mLuaFiles)
		{
			std::string key;
			if(!redisClient->LoadLuaScript(path, key))
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
#ifdef __DEBUG__
		this->InvokeCommand("FLUSHALL")->IsOk();
#endif
		this->SubscribeMessage();
		this->mTaskComponent->Start(&MainRedisComponent::StartPubSub, this);
		this->mTaskComponent->Start(&MainRedisComponent::CheckRedisClient, this);
		return true;
	}

	std::shared_ptr<RedisClient> MainRedisComponent::MakeRedisClient()
	{
#ifdef ONLY_MAIN_THREAD
		IAsioThread& workThread = App::Get()->GetTaskScheduler();
#else
		ThreadPoolComponent * threadPoolComponent = this->GetComponent<ThreadPoolComponent>();
		IAsioThread& workThread = threadPoolComponent->AllocateNetThread();
#endif
		const std::string & ip = this->mConfig.mIp;
		unsigned short port = this->mConfig.mPort;
		std::shared_ptr<SocketProxy> socketProxy = std::make_shared<SocketProxy>(workThread, ip, port);
		std::shared_ptr<RedisClient> redisCommandClient = std::make_shared<RedisClient>(socketProxy, this->mConfig);

		while(!redisCommandClient->StartConnect())
		{
			LOG_ERROR("connect redis [" << this->mConfig.mAddress << "] failure");
			this->mTaskComponent->Sleep(3000);
		}
		LOG_INFO("connect redis [" << this->mConfig.mAddress << "] successful");
		return redisCommandClient;
	}

	std::shared_ptr<RedisClient> MainRedisComponent::AllotRedisClient()
	{
		std::shared_ptr<RedisClient> redisClient;
		if (this->mFreeClients.empty())
		{
			std::shared_ptr<TaskSource<std::shared_ptr<RedisClient>>>
				taskSource(new TaskSource<std::shared_ptr<RedisClient>>());
			this->mWaitAllotClients.push(taskSource);
			redisClient = taskSource->Await();
		}
		else
		{
			redisClient = this->mFreeClients.front();
			this->mFreeClients.pop();
		}

		if (!redisClient->IsOpen() && !redisClient->StartConnect())
		{
			return nullptr;
		}
		return redisClient;
	}

	long long MainRedisComponent::Publish(const std::string& channel, const std::string& message)
	{
		return this->InvokeCommand("PUBLISH", channel, message)->GetNumber();
	}

	long long MainRedisComponent::Publish(const std::string address, const string& func, Json::Writer& jsonWriter)
	{
		jsonWriter.AddMember("func", func);
		return this->Publish(address, jsonWriter);
	}

	long long MainRedisComponent::Publish(const std::string& channel, Json::Writer& jsonWriter)
	{
		std::string json = jsonWriter.ToJsonString();
#ifdef __DEBUG__
		LOG_INFO("==== redis publish message ====");
		LOG_INFO("channel = " << channel);
		LOG_INFO("message = " << json);
#endif
		return this->Publish(channel, json);
	}

	void MainRedisComponent::CheckRedisClient()
	{

	}

	bool MainRedisComponent::SubscribeChannel(const std::string& channel)
	{
		LOG_CHECK_RET_FALSE(!channel.empty());
		std::shared_ptr<RedisRequest> request(new RedisRequest("SUBSCRIBE"));
		request->AddParameter(std::move(channel));
		std::shared_ptr<RedisResponse> response = this->mSubRedisClient->InvokeCommand(request)->Await();
		if(!response->HasError())
		{
			LOG_INFO("subscribe channel [" << channel << "] successful");
			return true;
		}
		LOG_INFO("subscribe channel [" << channel << "] failure : " << response->GetValue());
		return false;
	}

	void MainRedisComponent::SubscribeMessage()
	{
		std::vector<Component *> components;
		this->GetApp()->GetComponents(components);
		for(Component * component : components)
		{
			SubService * subService = component->Cast<SubService>();
			if(subService != nullptr && subService->IsStartService())
			{
				std::vector<std::string> methods;
				subService->GetSubMethods(methods);
				for (const std::string& name : methods)
				{
					const std::string& service = subService->GetName();
					this->SubscribeChannel(fmt::format("{0}.{1}", service, name));
				}
			}
		}
		this->SubscribeChannel(this->mRpcAddress);
	}

	void MainRedisComponent::StartPubSub()
	{
		while (this->mSubRedisClient)
		{
			if (!this->mSubRedisClient->IsOpen())
			{
				int count = 0;
				while(!this->mSubRedisClient->StartConnect())
				{
					LOG_ERROR("connect redis " << this->mConfig.mAddress << " failure count = " << count++);
					this->mTaskComponent->Sleep(3000);
				}
				this->SubscribeMessage();
				LOG_DEBUG("subscribe redis client connect successful");
			}

			std::shared_ptr<RedisResponse> redisResponse = this->mSubRedisClient->WaitRedisMessage()->Await();
			if (!redisResponse->HasError() && redisResponse->GetArraySize() == 3 && redisResponse->GetValue() == "message")
			{
				const std::string& channel = redisResponse->GetValue(1);
				const std::string& message = redisResponse->GetValue(2);
#ifdef __DEBUG__
				LOG_DEBUG("========= subscribe message =============");
				LOG_DEBUG("channel = " << channel);
				LOG_DEBUG("message = " << message);
#endif
				if (!this->HandlerSubMessage(channel, message))
				{
					LOG_FATAL(channel << " handler failure");
				}
			}
		}
	}

	bool MainRedisComponent::HandlerSubMessage(const std::string& channel, const std::string & message)
	{
		std::shared_ptr<Json::Reader> jsonReader(new Json::Reader());
		if (!jsonReader->ParseJson(message))
		{
			LOG_ERROR("parse sub message error");
			return false;
		}

		std::string service;
		std::string funcName;
		if (channel == this->mRpcAddress)
		{
			std::string fullName;
			jsonReader->GetMember("func", fullName);
			size_t pos = fullName.find('.');
			if (pos == std::string::npos)
			{
				LOG_ERROR(fullName << " parse error");
				return false;
			}
			service = fullName.substr(0, pos);
			funcName = fullName.substr(pos + 1);
		}
		else
		{
			size_t pos = channel.find('.');
			if (pos == std::string::npos)
			{
				LOG_ERROR(channel << " parse error");
				return false;
			}
			service = channel.substr(0, pos);
			funcName = channel.substr(pos + 1);
		}

		SubService* subService = this->GetComponent<SubService>(service);
		if (subService == nullptr)
		{
			LOG_ERROR("not find " << service);
			return false;
		}
		return subService->Invoke(funcName, *jsonReader);
	}

	long long MainRedisComponent::AddCounter(const string& key)
	{
		return this->InvokeCommand("INCR", key)->GetNumber();
	}

	bool MainRedisComponent::Lock(const string& key)
	{
		std::shared_ptr<RedisResponse> response = this->Call("lock.lock", key);
		if(response->IsOk())
		{
			//LOG_DEBUG("redis lock " << key << " get successful");
			this->mLockTimers[key] = this->mTimerComponent->DelayCall(4.5f, &MainRedisComponent::OnLockTimeout, this,
					key);
			return true;
		}
		//LOG_WARN("redis lock " << key << " get failure");
		return false;
	}

	bool MainRedisComponent::UnLock(const string& key)
	{
		this->Call("lock.unlock", key);
		auto iter = this->mLockTimers.find(key);
		if(iter != this->mLockTimers.end())
		{
			unsigned int id = iter->second;
			this->mLockTimers.erase(iter);
			this->mTimerComponent->CancelTimer(id);
		}
		LOG_DEBUG("redis lock " << key << " unlock");
		return true;
	}

	void MainRedisComponent::OnLockTimeout(const std::string& key)
	{
		auto iter = this->mLockTimers.find(key);
		if(iter != this->mLockTimers.end())
		{
			this->mLockTimers.erase(iter);
			this->mTaskComponent->Start([this, key]()
			{
				std::shared_ptr<RedisResponse> response = this->InvokeCommand("SETEX", key, 5, 1);
				if(response->IsOk())
				{
					//LOG_WARN("redis lock " << key << " delay successful");
					this->mLockTimers[key] = this->mTimerComponent->DelayCall(4.5f, &MainRedisComponent::OnLockTimeout,
							this, key);
					return ;
				}
				LOG_ERROR("redis lock " << key << " delay failure");
			});
		}
	}

}
