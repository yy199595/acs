#include"MainRedisComponent.h"

#include"App/App.h"
#include"Util/DirectoryHelper.h"
#include"Script/ClassProxyHelper.h"
#include"Global/ServiceConfig.h"
#include"Script/Extension/Redis/LuaRedis.h"
#include"Component/Scene/NetThreadComponent.h"
#include"DB/Redis/RedisClientContext.h"
#include"Component/Rpc/RpcHandlerComponent.h"
#include"Component/Scene/NetEventComponent.h"
namespace Sentry
{
	bool MainRedisComponent::LateAwake()
	{
        RedisComponent::LateAwake();
		this->mTaskComponent = this->GetComponent<TaskComponent>();
		this->mTimerComponent = this->GetComponent<TimerComponent>();
		LOG_CHECK_RET_FALSE(this->GetComponent<NetThreadComponent>());
		this->mRpcComponent = this->GetComponent<RpcHandlerComponent>();
		this->GetConfig().GetListener("rpc", this->mRpcAddress);
		return true;
	}

	bool MainRedisComponent::OnStart()
	{
        LOG_CHECK_RET_FALSE(RedisComponent::OnStart());
        this->mSubRedisClient = this->MakeRedisClient("main");
        if(this->mSubRedisClient != nullptr)
        {
            this->mSubRedisClient->EnableSubscribe();
        }
        if(!this->TryAsyncConnect(this->mSubRedisClient))
        {
            LOG_ERROR("start sub redis client error");
            return false;
        }
        return this->StartSubChannel();
	}

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
//        for(int index = 0; index < 100; index++)
//        {
//            this->mTaskComponent->Start([index, this]() {
//                ElapsedTimer timer;
//                for(int i = 0; i < 1; i++)
//                {
//                    this->Run("main", "SET", "test", index);
//                }
//                LOG_ERROR(index << " use time = " << timer.GetMs() << "ms");
//            });
//        }
        //this->mTaskComponent->WhenAll(tasks);
        return true;
	}

	long long MainRedisComponent::Publish(const std::string& channel, const std::string& message)
	{
        std::shared_ptr<RedisResponse> redisResponse = this->Run(
                this->mSubRedisClient, "PUBLISH", channel, message);
        return redisResponse->GetNumber();
	}

	bool MainRedisComponent::SubscribeChannel(const std::string& channel)
	{
        std::shared_ptr<RedisResponse> redisResponse =
                this->Run(this->mSubRedisClient, "SUBSCRIBE", channel);
        if(redisResponse->GetArraySize() == 3 && redisResponse->Get(2)->IsNumber())
        {
            if(((const RedisNumber*)redisResponse->Get(2))->GetValue() > 0)
            {
                LOG_INFO("sub " << channel << " successful");
                return true;
            }
        }
        LOG_INFO("sub " << channel << " failure");
        return false;
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
	void MainRedisComponent::OnLuaRegister(Lua::ClassProxyHelper& luaRegister)
	{
		luaRegister.BeginRegister<MainRedisComponent>();
		luaRegister.PushExtensionFunction("Run", Lua::Redis::Run);
		luaRegister.PushExtensionFunction("Call", Lua::Redis::Call);
	}
}
