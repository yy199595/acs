#include"MainRedisComponent.h"

#include"App/App.h"
#include"Util/DirectoryHelper.h"
#include"Script/ClassProxyHelper.h"
#include"Global/ServiceConfig.h"
#include"DB/Redis/RedisClientContext.h"
#include"Script/Extension/Redis/LuaRedis.h"
#include"Component/Scene/NetThreadComponent.h"
#include"Component/Rpc/ServiceRpcComponent.h"
#include"Component/Scene/NetEventComponent.h"
namespace Sentry
{
	bool MainRedisComponent::LateAwake()
	{
        RedisComponent::LateAwake();
		this->mTaskComponent = this->GetComponent<TaskComponent>();
		this->mTimerComponent = this->GetComponent<TimerComponent>();
		LOG_CHECK_RET_FALSE(this->GetComponent<NetThreadComponent>());
		this->mRpcTaskComponent = this->GetComponent<ServiceRpcComponent>();
		this->GetConfig().GetListener("rpc", this->mRpcAddress);
		return true;
	}

	bool MainRedisComponent::OnStart()
	{
		LOG_CHECK_RET_FALSE(RedisComponent::OnStart());

		std::shared_ptr<RedisRequest> request(new RedisRequest("LPUSH"));
		request->AddParameter("yjz11");

		for(size_t index = 0; index < 100; index++)
		{
			long long time = Helper::Time::GetNowSecTime() + index;
			request->AddParameter(Helper::Time::GetDateStr(time));
		}
		this->Run("main", request);
		std::shared_ptr<RedisResponse> response = this->Run("main", "LRANGE", "yjz11", 0, 100);


        this->mSubRedisClient = this->MakeRedisClient("main");
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
        this->mSubRedisClient->StartReceiveMessage();
        return true;
	}

    void MainRedisComponent::OnSecondUpdate(const int tick)
    {
        if (tick % 10 == 0 && this->mSubRedisClient != nullptr)
		{
			this->mSubRedisClient->SendCommand(std::make_shared<RedisRequest>("PING"));
		}
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

    void MainRedisComponent::OnCommandReply(SharedRedisClient redisClient, long long taskId, std::shared_ptr<RedisResponse> response)
    {
        auto iter = this->mTasks.find(taskId);
        if(iter == this->mTasks.end())
        {
            LOG_ERROR("not find redis task id = " << taskId);
            return;
        }
        iter->second->OnResponse(response);
        if(redisClient == this->mSubRedisClient)
        {
            this->mSubRedisClient->StartReceiveMessage();
        }
        this->mTasks.erase(iter);
    }

    std::shared_ptr<RedisTask> MainRedisComponent::AddRedisTask(std::shared_ptr<RedisRequest> request)
    {
		std::shared_ptr<RedisTask> task = request->MakeTask();
        long long taskId = task->GetRpcId();
        auto iter = this->mTasks.find(taskId);
        if(iter == this->mTasks.end())
        {
            this->mTasks.emplace(taskId, task);
            return task;
        }
        return nullptr;
    }

	std::shared_ptr<LuaRedisTask> MainRedisComponent::AddLuaRedisTask(std::shared_ptr<RedisRequest> request, lua_State * lua)
	{
		std::shared_ptr<LuaRedisTask> task = request->MakeLuaTask(lua);
		long long taskId = task->GetRpcId();
		auto iter = this->mTasks.find(taskId);
		if(iter == this->mTasks.end())
		{
			this->mTasks.emplace(taskId, task);
			return task;
		}
		return nullptr;
	}

    void MainRedisComponent::OnSubscribe(SharedRedisClient redisClient, const std::string &channel, const std::string &message)
    {
        if(!this->HandlerEvent(channel, message))
        {
            LOG_ERROR("handler " << channel << " error : " << message);
        }
        if(redisClient == this->mSubRedisClient)
        {
            this->mSubRedisClient->StartReceiveMessage();
        }
    }

	bool MainRedisComponent::HandlerEvent(const std::string& channel, const std::string& message)
	{
		if (message[0] == '+') //请求
		{
			const char* data = message.c_str() + 1;
			const size_t size = message.size() - 1;
			std::shared_ptr<com::rpc::request> request(new com::rpc::request());
			if (!request->ParseFromArray(data, size))
			{
				LOG_ERROR("parse message error");
				return false;
			}
			assert(!request->address().empty());
			this->mRpcTaskComponent->OnRequest(request);
			return true;
		}
		else if (message[0] == '-') //回复
		{
			const char* data = message.c_str() + 1;
			const size_t size = message.size() - 1;
			std::shared_ptr<com::rpc::response> response(new com::rpc::response());
			if (!response->ParseFromArray(data, size))
			{
				LOG_ERROR("parse message error");
				return false;
			}
            long long taskId = response->rpc_id();
			this->mRpcTaskComponent->OnResponse(taskId, response);
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
		jsonWriter << "key" << key << "time" << timeout;
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
