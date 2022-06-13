#include"MainRedisComponent.h"

#include"App/App.h"
#include"Util/DirectoryHelper.h"
#include"Script/ClassProxyHelper.h"
#include"Global/ServiceConfig.h"
#include"Component/Scene/NetThreadComponent.h"
#include"DB/Redis/RedisClientContext.h"
#include"Component/Rpc/RpcHandlerComponent.h"
#include"Component/Scene/NetEventComponent.h"
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
		this->GetConfig().GetListener("rpc", this->mRpcAddress);
		return true;
	}

	bool MainRedisComponent::LoadRedisConfig()
	{
		this->mConfig = this->GetConfig().GetRedisConfig("main");
		return this->mConfig != nullptr;
	}

	bool MainRedisComponent::OnStart()
	{
#if __REDIS_DEBUG__
		this->mDebugClient = this->MakeRedisClient(this->mConfig);
#endif
		this->mRedisClient = this->MakeRedisClient(this->mConfig);
		this->mSubRedisClient = this->MakeRedisClient(this->mConfig);
		if (this->mSubRedisClient == nullptr || this->mRedisClient == nullptr)
		{
			LOG_ERROR("connect redis " << this->mConfig->Address << " failure");
			return false;
		}

		for (const std::string& path: this->mConfig->LuaFiles)
		{
			if(!this->LoadLuaScript("", path))
			{
				LOG_ERROR("load " << path << " failure");
				return false;
			}
		}

		LOG_CHECK_RET_FALSE(this->StartSubChannel());
#if __REDIS_DEBUG__
		this->mTaskComponent->Start(&MainRedisComponent::StartDebugRedis, this);
#endif
		this->mTaskComponent->Start(&MainRedisComponent::StartPubSub, this);
		this->mTaskComponent->Start(&MainRedisComponent::CheckRedisClient, this);
		return true;
	}
#if __REDIS_DEBUG__
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
				if(!this->TryReConnect(this->mDebugClient))
				{
					LOG_ERROR("debug redis client connect failure");
					return;
				}
				LOG_DEBUG("debug redis client connect successful");
			}
			response->Clear();
			if(this->mDebugClient->WaitRedisResponse(response) == XCode::Successful)
			{
				std::string content;
				if(response->GetString(content))
				{
					LOG_DEBUG("[redis command] = " << content);
				}
			}
		}
	}
#endif

	bool MainRedisComponent::StartSubChannel()
	{
		if (!this->SubscribeChannel(this->mRpcAddress))
		{
			return false;
		}
		std::vector<Component*> components;
		this->GetApp()->GetComponents(components);
		for (Component* component: components)
		{
			NetEventComponent* localServiceComponent = component->Cast<NetEventComponent>();
			if (localServiceComponent != nullptr)
			{
				if(!localServiceComponent->StartRegisterEvent())
				{
					LOG_INFO(component->GetName() << " start listen event failure");
					return false;
				}
				LOG_INFO(component->GetName() << " start listen event successful");
			}
		}
		return true;
	}

	long long MainRedisComponent::Publish(const std::string& channel, const std::string& message)
	{
		if(!this->mRedisClient->IsOpen())
		{
			this->TryReConnect(this->mRedisClient);
		}
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
#ifndef __DEBUG__
		while (this->mSubRedisClient != nullptr)
		{
			this->mTaskComponent->Sleep(1000);
			std::shared_ptr<RedisRequest> ping = RedisRequest::Make("PING");
			if(this->mSubRedisClient->Run(ping) != XCode::Successful)
			{

			}
		}
#endif
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
		std::string type;
		std::shared_ptr<RedisResponse> redisResponse(new RedisResponse());
		while (this->mSubRedisClient)
		{
			redisResponse->Clear();
			if (!this->mSubRedisClient->IsOpen())
			{
				if(!this->TryReConnect(this->mSubRedisClient))
				{
					LOG_FATAL("try reconnect sub redis failure");
					return;
				}
				int count = 0;
				LOG_DEBUG("subscribe redis client connect successful");
				while(!this->StartSubChannel())
				{
					this->mTaskComponent->Sleep(3000);
					LOG_ERROR("sub client sub chanel failure count = " << count++);
				}
			}
			XCode code = this->mSubRedisClient->WaitRedisResponse(redisResponse);
			if (code != XCode::Successful || redisResponse->HasError())
			{
				LOG_FATAL("sub redis client error");
				continue;
			}

			if (redisResponse->GetArraySize() == 3 && redisResponse->GetString(type) && type == "message")
			{
				std::string channel, message;
				redisResponse->GetString(channel, 1);
				redisResponse->GetString(message, 2);
				if(!this->HandlerEvent(channel, message))
				{
					LOG_ERROR("handler " << channel << " error : " << message);
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
			assert(!request->address().empty());
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
		std::shared_ptr<Json::Reader> jsonReader(new Json::Reader());
		if (!jsonReader->ParseJson(message))
		{
			return false;
		}
		NetEventComponent* localServiceComponent = this->GetComponent<NetEventComponent>(channel);
		if(localServiceComponent == nullptr)
		{
			std::string service;
			LOG_CHECK_RET_FALSE(jsonReader->GetMember("service", service));
			localServiceComponent = this->GetComponent<NetEventComponent>(service);
		}
		std::string eveId;
		LOG_CHECK_RET_FALSE(jsonReader->GetMember("eveId", eveId));
		return localServiceComponent != nullptr && localServiceComponent->Invoke(eveId, jsonReader);
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
		if(!this->Call("lock.lock", jsonWriter))
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

	std::shared_ptr<RedisClientContext> MainRedisComponent::GetClient(const std::string& name)
	{
		if(!this->mRedisClient->IsOpen())
		{
			if(!this->TryReConnect(this->mRedisClient))
			{
				return nullptr;
			}
		}
		return this->mRedisClient;
	}
}
