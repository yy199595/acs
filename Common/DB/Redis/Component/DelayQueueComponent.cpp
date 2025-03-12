//
// Created by yy on 2024/5/12.
//

#include "XCode/XCode.h"
#include "DelayQueueComponent.h"
#include "Util/Tools/TimeHelper.h"
#include "Redis/Component/RedisComponent.h"

namespace acs
{
	DelayQueueComponent::DelayQueueComponent()
	{
		this->mRedis = nullptr;
	}

	bool DelayQueueComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mRedis = this->GetComponent<RedisComponent>())
		return true;
	}

	std::vector<std::string> DelayQueueComponent::List(const std::string& key)
	{
		long long nowTime = help::Time::NowSec();
		const std::string name = fmt::format("delay_queue:{}", key);
		std::unique_ptr<redis::Response> response = this->mRedis->Run("ZRANGEBYSCORE", name, 0, nowTime);
		if(response == nullptr || response->element.type != redis::type::Array)
		{
			return std::vector<std::string>();
		}
		std::vector<std::string> result;
		result.reserve(response->element.list.size());
		for(const redis::Element & element : response->element.list)
		{
			result.emplace_back(element.message);
		}
		return result;
	}

	int DelayQueueComponent::Del(const std::string& key, const std::string& value)
	{
		const std::string name = fmt::format("delay_queue:{}", key);
		std::unique_ptr<redis::Response> response = this->mRedis->Run("ZREM", name, value);
		if(response == nullptr || response->HasError())
		{
			return XCode::Failure;
		}
		return response->element.number > 0;
	}

	int DelayQueueComponent::Del(const std::string& key, const std::vector<std::string>& members)
	{
		if(members.empty())
		{
			return XCode::Ok;
		}
		const std::string name = fmt::format("delay_queue:{}", key);
		std::unique_ptr<redis::Request> request = std::make_unique<redis::Request>();
		{
			request->SetCommand("ZREM");
			request->AddParameter(name);
			for(const std::string & value : members)
			{
				request->AddParameter(value);
			}
		}
		std::unique_ptr<redis::Response> response = this->mRedis->Run(std::move(request));
		if(response == nullptr || response->HasError())
		{
			return XCode::Failure;
		}
		return response->element.number > 0 ? XCode::Ok : XCode::Failure;
	}

	int DelayQueueComponent::Add(const std::string& key, const std::string& value, int second)
	{
		long long score = help::Time::NowSec() + second;
		const std::string name = fmt::format("delay_queue:{}", key);
		std::unique_ptr<redis::Response> response = this->mRedis->Run("ZADD", name, score, value);
		if(response == nullptr || response->HasError())
		{
			return XCode::Failure;
		}
		return response->element.number > 0 ? XCode::Ok : XCode::Failure;
	}

	int DelayQueueComponent::Add(const std::string& key, const std::vector<std::string>& value, int second)
	{
		if(value.empty())
		{
			return XCode::Ok;
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
		std::unique_ptr<redis::Response> response = this->mRedis->Run(std::move(request));
		if(response == nullptr || response->HasError())
		{
			return XCode::Failure;
		}
		return response->element.number > 0 ? XCode::Ok : XCode::Failure;
	}

}