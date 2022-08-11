//
// Created by mac on 2022/5/18.
//

#include "RedisNode.h"

namespace Sentry
{
	RedisNode::RedisNode(const RedisConfig* config)
		: mConfig(config)
	{
		this->mIndex = 0;
	}

	void RedisNode::AddClient(std::shared_ptr<RedisRpcClient> client)
	{
		this->mClients.push_back(client);
	}

	void RedisNode::CheckAllClient(long long nowTime)
	{
		auto iter = this->mClients.begin();
		for(; iter != this->mClients.end();)
		{
			if(nowTime - (*iter)->GetLastOperTime() >= 60) // 60秒未使用
			{
				iter = this->mClients.erase(iter++);
				LOG_WARN(this->mConfig->Name << " remove free client");
				continue;
			}
			iter++;
		}
	}

	std::shared_ptr<RedisRpcClient> RedisNode::GetFreeClient()
	{
		if(this->mClients.empty())
		{
			return nullptr;
		}
		for(std::shared_ptr<RedisRpcClient> redisClientContext : this->mClients)
		{
			//if(!redisClientContext->IsUse())
			{
				return redisClientContext;
			}
		}
		if(this->mClients.size() >= this->mConfig->Count)
		{
			if(this->mIndex >= this->mClients.size())
			{
				this->mIndex = 0;
			}
			return this->mClients[this->mIndex++];
		}
		return nullptr;
	}
}