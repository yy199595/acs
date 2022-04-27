//
// Created by yjz on 2022/4/26.
//

#include"DataRedisComponent.h"
#include"Component/Scene/ThreadPoolComponent.h"
namespace Sentry
{
	bool DataRedisComponent::LateAwake()
	{
		this->GetConfig().GetRedisConfigs(this->mRedisConfigs);
		LOG_CHECK_RET_FALSE(this->GetComponent<ThreadPoolComponent>());
		return this->mRedisConfigs.size() >= 2;
	}

	bool DataRedisComponent::OnStart()
	{
		for (const RedisConfig* config: this->mRedisConfigs)
		{
			if (config->Name == "main")
			{
				continue;
			}
			for (int index = 0; index < config->Count; index++)
			{
				std::shared_ptr<RedisClient> redisClient = this->MakeRedisClient(config);
				if (redisClient == nullptr)
				{
					return false;
				}
				this->mFreeClients.emplace(redisClient);
			}
		}
		return true;
	}

	std::shared_ptr<RedisClient> DataRedisComponent::MakeRedisClient(const RedisConfig * config)
	{
#ifdef ONLY_MAIN_THREAD
		IAsioThread& workThread = App::Get()->GetTaskScheduler();
#else
		ThreadPoolComponent * threadPoolComponent = this->GetComponent<ThreadPoolComponent>();
		IAsioThread& workThread = threadPoolComponent->AllocateNetThread();
#endif
		const std::string & ip = config->Ip;
		unsigned short port = config->Port;
		std::shared_ptr<SocketProxy> socketProxy = std::make_shared<SocketProxy>(workThread, ip, port);
		std::shared_ptr<RedisClient> redisCommandClient = std::make_shared<RedisClient>(socketProxy, config);

		while(!redisCommandClient->StartConnect())
		{
			LOG_ERROR(config->Name << " connect redis [" << config->Address << "] failure");
			this->mTaskComponent->Sleep(3000);
		}
		LOG_INFO(config->Name << " connect redis [" << config->Address << "] successful");
		return redisCommandClient;
	}

}
