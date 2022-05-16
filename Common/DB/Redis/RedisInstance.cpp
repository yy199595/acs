//
// Created by mac on 2022/5/6.
//

#include "RedisInstance.h"
#include"Component/Scene/ThreadPoolComponent.h"
namespace Sentry
{
	RedisInstance::RedisInstance(const RedisConfig * config)
		: mConfig(config)
	{
		this->mTaskComponent = App::Get()->GetTaskComponent();
	}

	bool RedisInstance::StartConnect()
	{
		for (int index = 0; index < this->mConfig->Count; index++)
		{
			std::shared_ptr<RedisClientContext> redisClientContext = this->MakeRedisClient();
			if(redisClientContext == nullptr)
			{
				return false;
			}
			this->mFreeClients.push(redisClientContext);
		}
		return true;
	}

	std::shared_ptr<RedisClientContext> RedisInstance::MakeRedisClient()
	{
#ifdef ONLY_MAIN_THREAD
		IAsioThread& workThread = App::Get()->GetTaskScheduler();
#else

		ThreadPoolComponent * threadPoolComponent = App::Get()->GetComponent<ThreadPoolComponent>();
		IAsioThread& workThread = threadPoolComponent->AllocateNetThread();
#endif
		const std::string& ip = this->mConfig->Ip;
		unsigned short port = this->mConfig->Port;
		std::shared_ptr<SocketProxy> socketProxy = std::make_shared<SocketProxy>(workThread, ip, port);
		std::shared_ptr<RedisClientContext> redisCommandClient = std::make_shared<RedisClientContext>(socketProxy, this->mConfig);

		XCode code = redisCommandClient->StartConnect();
		if (code == XCode::RedisAuthFailure)
		{
			LOG_ERROR(this->mConfig->Address << " auth failure");
			return nullptr;
		}

		while (code != XCode::Successful)
		{
			LOG_ERROR(this->mConfig->Name << " connect redis [" << this->mConfig->Address << "] failure");
			this->mTaskComponent->Sleep(3000);
			code = redisCommandClient->StartConnect();
		}
		LOG_INFO(this->mConfig->Name << " connect redis [" << this->mConfig->Address << "] successful");
		return redisCommandClient;
	}

	std::shared_ptr<RedisClientContext> RedisInstance::GetRedisClient()
	{
		std::shared_ptr<RedisClientContext> redisClientContext = this->mFreeClients.front();
		this->mFreeClients.pop();
		this->mFreeClients.push(redisClientContext);
		return redisClientContext;
	}


}