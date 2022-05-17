//
// Created by yjz on 2022/4/26.
//

#include"DataRedisComponent.h"
#include"Component/Scene/NetThreadComponent.h"
namespace Sentry
{
	bool DataRedisComponent::LateAwake()
	{
		this->GetConfig().GetRedisConfigs(this->mRedisConfigs);
		LOG_CHECK_RET_FALSE(this->GetComponent<NetThreadComponent>());
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
			std::shared_ptr<RedisInstance> redisInstance(new RedisInstance(config));
			if (!redisInstance->StartConnect())
			{
				return false;
			}
			this->mRedisInsts.emplace(config->Name, redisInstance);
		}
		return true;
	}

	std::shared_ptr<RedisClientContext> DataRedisComponent::GetRedisClient(const std::string& name)
	{
		auto iter = this->mRedisInsts.find(name);
		if (iter == this->mRedisInsts.end())
		{
			return nullptr;
		}
		return iter->second->GetRedisClient();
	}
}
