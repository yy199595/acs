//
// Created by yjz on 2022/4/26.
//

#include"DataRedisComponent.h"
#include"Component/Scene/ThreadPoolComponent.h"
namespace Sentry
{
	bool DataRedisComponent::LateAwake()
	{
		this->mConfig.mCount = 3;
		this->mTaskComponent = this->GetComponent<TaskComponent>();
		this->mTimerComponent = this->GetComponent<TimerComponent>();
		LOG_CHECK_RET_FALSE(this->GetComponent<ThreadPoolComponent>());
		this->GetConfig().GetMember("redis.data", "count", this->mConfig.mCount);
		this->GetConfig().GetMember("redis.data", "lua", this->mConfig.mLuaFiles);
		this->GetConfig().GetMember("redis.data", "passwd", this->mConfig.mPassword);
		LOG_CHECK_RET_FALSE(this->GetConfig().GetMember("redis.data", "ip", this->mConfig.mIp));
		LOG_CHECK_RET_FALSE(this->GetConfig().GetMember("redis.data", "port", this->mConfig.mPort));
		return true;
	}

	bool DataRedisComponent::OnStart()
	{
		for(int index = 0; index < this->mConfig.mCount; index++)
		{
			std::shared_ptr<RedisClient> redisClient = this->MakeRedisClient();
			if(redisClient == nullptr)
			{
				return false;
			}
			this->mFreeClients.emplace(redisClient);
		}
		return true;
	}
	std::shared_ptr<RedisClient> DataRedisComponent::MakeRedisClient()
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
	std::shared_ptr<RedisResponse> DataRedisComponent::Invoke(const string& cmd, vector<std::string>& parameter)
	{
		return nullptr;
	}
}
