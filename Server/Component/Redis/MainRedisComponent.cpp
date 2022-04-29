#include"MainRedisComponent.h"

#include"App/App.h"
#include"Util/FileHelper.h"
#include"Util/DirectoryHelper.h"
#include"Script/ClassProxyHelper.h"
#include"Component/Service/SubService.h"
#include"Global/RpcConfig.h"
#include"Component/Scene/ThreadPoolComponent.h"
#include"DB/Redis/RedisClientContext.h"
namespace Sentry
{
	bool MainRedisComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->LoadRedisConfig());
		this->mTaskComponent = this->GetComponent<TaskComponent>();
		this->mTimerComponent = this->GetComponent<TimerComponent>();
		LOG_CHECK_RET_FALSE(this->GetComponent<ThreadPoolComponent>());
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
		if(this->mRedisClient->Run(request, response) != XCode::Successful || !response->IsOk())
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

		LOG_CHECK_RET_FALSE(this->SubscribeMessage());
		this->mTaskComponent->Start(&MainRedisComponent::StartPubSub, this);
		this->mTaskComponent->Start(&MainRedisComponent::CheckRedisClient, this);
		return true;
	}

	std::shared_ptr<RedisClientContext> MainRedisComponent::MakeRedisClient()
	{
#ifdef ONLY_MAIN_THREAD
		IAsioThread& workThread = App::Get()->GetTaskScheduler();
#else
		ThreadPoolComponent * threadPoolComponent = this->GetComponent<ThreadPoolComponent>();
		IAsioThread& workThread = threadPoolComponent->AllocateNetThread();
#endif
		size_t count = 0;
		const std::string & ip = this->mConfig->Ip;
		unsigned short port = this->mConfig->Port;
		std::shared_ptr<SocketProxy> socketProxy = std::make_shared<SocketProxy>(workThread, ip, port);
		std::shared_ptr<RedisClientContext> redisCommandClient = std::make_shared<RedisClientContext>(socketProxy, this->mConfig);

		XCode code = redisCommandClient->StartConnect();
		if(code == XCode::RedisAuthFailure)
		{
			LOG_ERROR("redis auth failure");
			return nullptr;
		}

		while(code != XCode::Successful)
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
		if(this->mRedisClient->Run(request, response) != XCode::Successful)
		{
			LOG_ERROR("redis net work error publish error");
			return 0;
		}
		return response->GetNumber();
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
		while(this->mSubRedisClient != nullptr)
		{
			this->mTaskComponent->Sleep(5000);
			std::shared_ptr<RedisRequest> request = RedisRequest::Make("HGET", "lua", "json.lua");
			std::shared_ptr<RedisResponse> response = std::make_shared<RedisResponse>();
			if(this->mRedisClient->Run(request, response) != XCode::Successful)
			{

			}
		}
	}

	bool MainRedisComponent::SubscribeChannel(const std::string& channel)
	{
		assert(!channel.empty());
		std::shared_ptr<RedisResponse> response = std::make_shared<RedisResponse>();
		std::shared_ptr<RedisRequest> request = RedisRequest::Make("SUBSCRIBE", channel);
		if(this->mSubRedisClient->Run(request, response) != XCode::Successful)
		{
			LOG_ERROR("redis net work error sub " << channel << " failure");
			return false;
		}
		LOG_INFO("subscribe channel [" << channel << "] successful");
		return true;
	}

	bool MainRedisComponent::SubscribeMessage()
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
					if(!this->SubscribeChannel(fmt::format("{0}.{1}", service, name)))
					{
						return false;
					}
				}
			}
		}
		return this->SubscribeChannel(this->mRpcAddress);
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
				this->SubscribeMessage();
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
				if (message[0] == '+') //请求
				{
					XCode code = XCode::ParseJsonFailure;
					std::shared_ptr<Json::Writer> jsonResponse(new Json::Writer());
					std::shared_ptr<com::Sub::Request> request(new com::Sub::Request());
					std::shared_ptr<com::Sub::Response> response(new com::Sub::Response());
					if (request->ParseFromArray(message.c_str() + 1, message.size() - 1))
					{
						response->set_rpc_id(request->rpc_id());
						code = this->OnRequest(*request, *jsonResponse);
					}
					std::string result = "-";
					response->set_code((int)code);
					response->set_json(jsonResponse->ToJsonString());
					if (response->AppendPartialToString(&result))
					{
						this->Publish(request->channel(), result);
					}
				}
				else if (message[0] == '-') //回复
				{
					std::shared_ptr<com::Sub::Response> response(new com::Sub::Response());
					if (response->ParseFromArray(message.c_str() + 1, message.size() - 1))
					{
						this->OnResponse(*response);
					}
				}
				else if(message[0] == '=') // 广播
				{

				}
			}
		}

	}

	XCode MainRedisComponent::OnRequest(const com::Sub::Request& request, Json::Writer& response)
	{

	}

	XCode MainRedisComponent::OnResponse(com::Sub::Response& response)
	{

	}


	XCode MainRedisComponent::HandlerSubMessage(std::shared_ptr<Json::Reader> request, std::shared_ptr<Json::Writer>& response)
	{
		std::string fullName;
		request->GetMember("func", fullName);
		size_t pos = fullName.find('.');
		if (pos == std::string::npos)
		{
			LOG_ERROR(fullName << " parse error");
			return XCode::CallFunctionNotExist;
		}
		std::string service = fullName.substr(0, pos);
		std::string funcName = fullName.substr(pos + 1);
		SubService* subService = this->GetComponent<SubService>(service);
		if (subService == nullptr)
		{
			return XCode::CallServiceNotFound;
		}
		return subService->Invoke(funcName, *request, *response);
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

	bool MainRedisComponent::Lock(const string& key)
	{
		std::string tag;
		if (!this->GetLuaScript("lock.lua", tag))
		{
			LOG_ERROR("not find redis script lock.lua");
			return false;
		}
		std::unordered_map<std::string, std::string> parateters
			{
				std::make_pair("key", key),
				std::make_pair("time", "5")
			};
		std::shared_ptr<RedisResponse> response(new RedisResponse());
		std::shared_ptr<RedisRequest> request = RedisRequest::MakeLua(tag, "lock", parateters);
		if (this->mRedisClient->Run(request, response) != XCode::Successful || !response->IsOk())
		{
			return false;
		}
		//LOG_DEBUG("redis lock " << key << " get successful");
		this->mLockTimers[key] = this->mTimerComponent->DelayCall(4.5f, &MainRedisComponent::OnLockTimeout, this, key);
		return true;
	}

	bool MainRedisComponent::UnLock(const string& key)
	{
		std::string tag;
		if (!this->GetLuaScript("lock.lua", tag))
		{
			LOG_ERROR("not find redis script lock.lua");
			return false;
		}
		std::unordered_map<std::string, std::string> parateters
			{
				std::make_pair("key", key),
				std::make_pair("time", "5")
			};
		std::shared_ptr<RedisResponse> response1(new RedisResponse());
		std::shared_ptr<RedisRequest> request1 = RedisRequest::MakeLua(tag, "unlock", parateters);
		if (this->mRedisClient->Run(request1, response1) != XCode::Successful || !response1->IsOk())
		{

		}

		auto iter = this->mLockTimers.find(key);
		if(iter != this->mLockTimers.end())
		{
			unsigned int id = iter->second;
			this->mLockTimers.erase(iter);
			this->mTimerComponent->CancelTimer(id);
		}
		std::shared_ptr<RedisRequest> request = RedisRequest::Make("DEL", key);
		std::shared_ptr<RedisResponse> response = std::make_shared<RedisResponse>();
		if(this->mRedisClient->Run(request, response) != XCode::Successful)
		{
			return false;
		}
		LOG_DEBUG("redis lock " << key << " unlock");
		return true;
	}

	void MainRedisComponent::OnLockTimeout(const std::string& key)
	{
		auto iter = this->mLockTimers.find(key);
		if (iter != this->mLockTimers.end())
		{
			this->mLockTimers.erase(iter);
			this->mTaskComponent->Start([this, key]()
			{
				std::shared_ptr<RedisResponse> response(new RedisResponse());
				std::shared_ptr<RedisRequest> request = RedisRequest::Make("SETEX", key, 5, 1);
				if(this->mRedisClient->Run(request, response) == XCode::Successful && response->IsOk())
				{
					this->mLockTimers[key] = this->mTimerComponent->DelayCall(4.5f, &MainRedisComponent::OnLockTimeout,
							this, key);
					return;
				}
				LOG_ERROR("redis lock " << key << " delay failure");
			});
		}
	}

}
