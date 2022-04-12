//
// Created by yjz on 2022/1/22.
//
#include"SubService.h"
#include"Component/Redis/RedisComponent.h"
namespace Sentry
{
	bool SubService::LateAwake()
	{
		this->mRedisComponent = this->GetComponent<RedisComponent>();
		return true;
	}
	void SubService::GetSubMethods(std::vector<std::string>& methods)
	{
		if(this->mServiceRegister != nullptr)
		{
			this->mServiceRegister->GetMethods(methods);
		}
	}

	bool SubService::Publish(const std::string& func, Json::Writer& jsonWriter)
	{
		if(this->mRedisComponent != nullptr)
		{
			return this->mRedisComponent->Publish(func, jsonWriter) > 0;
		}
		return false;
	}

	bool SubService::Publish(const std::string& address, const std::string& func, Json::Writer& jsonWriter)
	{
		if(this->mRedisComponent != nullptr)
		{
			jsonWriter.AddMember("func", func);
			return this->mRedisComponent->Publish(address, jsonWriter) > 0;
		}
		return false;
	}

	bool SubService::Invoke(const std::string& func, const Json::Reader & jsonReader)
	{
		if(this->mServiceRegister == nullptr)
		{
			return false;
		}
		std::shared_ptr<SubMethod> subMethod = this->mServiceRegister->GetMethod(func);
		if(subMethod != nullptr)
		{
			subMethod->OnPublish(jsonReader);
			return true;
		}
		return false;
	}

	bool SubService::LoadService()
	{
		this->mServiceRegister = std::make_shared<SubServiceRegister>(this);
		return this->OnInitService(*this->mServiceRegister);
	}

	void SubService::OnAddAddress(const std::string& address)
	{

	}
}
