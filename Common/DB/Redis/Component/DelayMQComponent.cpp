//
// Created by yy on 2024/5/12.
//
#include "DelayMQComponent.h"
#include "Util/Tools/TimeHelper.h"
#include "Redis/Component/RedisComponent.h"

namespace acs
{
	DelayMQComponent::DelayMQComponent()
	{
		this->mRedis = nullptr;
	}

	bool DelayMQComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mRedis = this->GetComponent<RedisComponent>())
		return true;
	}

	std::vector<std::string> DelayMQComponent::List(const std::string& key)
	{
		std::vector<std::string> result;
		do
		{
			long long nowTime = help::Time::NowSec();
			const std::string name = fmt::format("delay_queue:{}", key);
			std::unique_ptr<redis::Response> response = this->mRedis->Run("ZRANGEBYSCORE", name, 0, nowTime);
			if (response == nullptr || response->element.type != redis::type::Array)
			{
				break;
			}
			result.reserve(response->element.list.size());
			for (const redis::Element& element: response->element.list)
			{
				result.emplace_back(element.message);
			}
		}
		while(false);
		return result;
	}

	bool DelayMQComponent::Del(const std::string& key, const std::string& value)
	{
		const std::string name = fmt::format("delay_queue:{}", key);
		std::unique_ptr<redis::Response> response = this->mRedis->Run("ZREM", name, value);
		if(response == nullptr || response->HasError())
		{
			return false;
		}
		return response->element.number > 0;
	}

	bool DelayMQComponent::Del(const std::string& key, const std::vector<std::string>& members)
	{
		if(members.empty())
		{
			return true;
		}
		const std::string name = fmt::format("delay_queue:{}", key);
		std::unique_ptr<redis::Response> response = this->mRedis->Run("ZREM", name, members);
		if(response == nullptr || response->HasError())
		{
			return false;
		}
		return response->element.number > 0;
	}

	bool DelayMQComponent::Add(const std::string& key, const std::string& value, int second)
	{
		long long score = help::Time::NowSec() + second;
		const std::string name = fmt::format("delay_queue:{}", key);
		std::unique_ptr<redis::Response> response = this->mRedis->Run("ZADD", name, score, value);
		if(response == nullptr || response->HasError())
		{
			return false;
		}
		return response->element.number > 0;
	}

	bool DelayMQComponent::Add(const std::string& key, const std::vector<std::string>& value, int second)
	{
		if(value.empty())
		{
			return true;
		}
		long long score = help::Time::NowSec() + second;
		const std::string name = fmt::format("delay_queue:{}", key);
		std::unique_ptr<redis::Request> request = std::make_unique<redis::Request>();
		{
			request->SetCommand("ZADD");
			request->AddParameter(name);
			for(const std::string & member : value)
			{
				request->AddParameter(score);
				request->AddParameter(member);
			}
		}
		std::unique_ptr<redis::Response> response = this->mRedis->Run(request);
		if(response == nullptr || response->HasError())
		{
			return false;
		}
		return response->element.number > 0;
	}

}