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
        RedisComponent::LateAwake();
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
        LOG_CHECK_RET_FALSE(RedisComponent::OnStart());
        this->mSubRedisClient = this->GetClient("main");
        return this->mSubRedisClient != nullptr && this->StartSubChannel();
	}

	bool MainRedisComponent::StartSubChannel()
	{
        this->mSubRedisClient->EnableSubscribe();
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
        std::shared_ptr<RedisRequest> request = RedisRequest::Make("PUBLISH", channel, message);
        std::shared_ptr<RedisTask> redisTask = request->MakeTask();
        if(!this->AddRedisTask(redisTask))
        {
            return -1;
        }
        if(!this->TryAsyncConnect(this->mSubRedisClient))
        {
            LOG_ERROR("redis net work error publish error");
            return -1;
        }
        this->mSubRedisClient->SendCommand(request);
		return redisTask->Await()->GetNumber();
	}

	bool MainRedisComponent::SubscribeChannel(const std::string& channel)
	{
		std::shared_ptr<RedisRequest> request = RedisRequest::Make("SUBSCRIBE", channel);
        std::shared_ptr<RedisTask> redisTask = request->MakeTask();
        if(!this->AddRedisTask(redisTask))
        {
            return false;
        }
        if(!this->TryAsyncConnect(this->mSubRedisClient))
        {
            LOG_ERROR("redis net work error sub " << channel << " failure");
            return false;
        }
        this->mSubRedisClient->SendCommand(request);
        return redisTask->Await()->GetNumber() > 0;
	}

    void MainRedisComponent::OnCommandReply(std::shared_ptr<RedisResponse> response)
    {
        long long taskId = response->GetTaskId();
        auto iter = this->mTasks.find(taskId);
        if(iter == this->mTasks.end())
        {
            LOG_ERROR("not find redis task id = " << response->GetTaskId());
            return;
        }
        iter->second->OnResponse(response);
        this->mTasks.erase(iter);
    }

    bool MainRedisComponent::AddRedisTask(std::shared_ptr<IRpcTask<RedisResponse>> task)
    {
        long long taskId = task->GetRpcId();
        auto iter = this->mTasks.find(taskId);
        if(iter == this->mTasks.end())
        {
            this->mTasks.emplace(taskId, task);
            return true;
        }
        return false;
    }

    void MainRedisComponent::OnSubscribe(const std::string &channel, const std::string &message)
    {
        if(!this->HandlerEvent(channel, message))
        {
            LOG_ERROR("handler " << channel << " error : " << message);
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

	bool MainRedisComponent::Lock(const string& key, int timeout)
	{
		Json::Writer jsonWriter;
		jsonWriter.AddMember("key", key);
		jsonWriter.AddMember("time", timeout);
		if(!this->Call("main", "lock.lock", jsonWriter))
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
		std::shared_ptr<RedisResponse> response1 = this->Run("main", "DEL", key);
        if(response1->GetNumber() != 1)
        {
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
				std::shared_ptr<RedisResponse> response = this->Run("main", "SETEX", 10, 1);
                if(response != nullptr && response->IsOk())
                {
                    this->mLockTimers[key] = this->mTimerComponent->DelayCall(
                            (float)timeout - 0.5f, &MainRedisComponent::OnLockTimeout,this, key, timeout);
                    return;
                }
				LOG_ERROR("redis lock " << key << " delay failure");
			});
		}
	}
}
