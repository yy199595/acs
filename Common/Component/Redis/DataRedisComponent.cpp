//
// Created by yjz on 2022/4/26.
//

#include"DataRedisComponent.h"
#include"Util/TimeHelper.h"
#include"Component/Scene/NetThreadComponent.h"
namespace Sentry
{
	bool DataRedisComponent::LateAwake()
	{
		this->mTick = 0;
		this->GetConfig().GetRedisConfigs(this->mRedisConfigs);
		LOG_CHECK_RET_FALSE(this->GetComponent<NetThreadComponent>());
		return this->mRedisConfigs.size() >= 2;
	}

	bool DataRedisComponent::OnStart()
	{
		for (const RedisConfig* config: this->mRedisConfigs)
		{
			std::vector<std::shared_ptr<RedisClientContext>> redisArray;
			if (config->Name != "main")
			{
				std::shared_ptr<RedisNode> redisNode(new RedisNode(config));
				std::shared_ptr<RedisClientContext> redisClientContext = this->MakeRedisClient(config);
				if (redisClientContext == nullptr)
				{
					LOG_ERROR("connect redis [" << config->Name << " " << config->Address << "] failure");
					return false;
				}
				this->mRedisNodes.emplace(config->Name, redisNode);
				this->mRedisNodes[config->Name]->AddClient(redisClientContext);
			}
		}
		return true;
	}

	void DataRedisComponent::OnSecondUpdate()
	{
		this->mTick++;
		if(this->mTick >= 30)
		{
			long long now = Time::GetNowSecTime();
			auto iter = this->mRedisNodes.begin();
			for(; iter != this->mRedisNodes.end(); iter++)
			{
				iter->second->CheckAllClient(now);
			}
			this->mTick = 0;
		}
	}

	std::shared_ptr<RedisClientContext> DataRedisComponent::GetClient(const std::string& name)
	{
		auto iter = this->mRedisNodes.find(name);
		if (iter == this->mRedisNodes.end())
		{
			return nullptr;
		}
		std::shared_ptr<RedisNode> redisNode = iter->second;
		std::shared_ptr<RedisClientContext> redisClientContext = redisNode->GetFreeClient();
		if(redisClientContext == nullptr)
		{
			redisClientContext = this->MakeRedisClient(redisNode->GetConfig());
			if(redisClientContext != nullptr)
			{
				redisNode->AddClient(redisClientContext);
			}
		}
		return redisClientContext;
	}
}
