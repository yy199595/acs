//
// Created by yy on 2024/4/27.
//
#include "XCode/XCode.h"
#include "OperationComponent.h"
#include "Redis/Component/RedisComponent.h"
namespace joke
{
	OperationComponent::OperationComponent()
	{
		this->mRedis = nullptr;
	}

	bool OperationComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mRedis = this->GetComponent<RedisComponent>())
		return true;
	}

	int OperationComponent::Add(int city, int type, const char* field, int value)
	{
		if(value <= 0)
		{
			return XCode::Ok;
		}
		const std::string key = fmt::format("operation:{}", city);
		const std::string fieldName = fmt::format("{}:{}", field, type);
		redis::Response * response = this->mRedis->Run("HINCRBY", key, fieldName, value);
		if(response == nullptr || response->HasError())
		{
			LOG_ERROR("save operation fail city:{} type:{} field:{} value:{}", city, type, field, value);
			return XCode::SaveToRedisFailure;
		}
		return XCode::Ok;
	}

}