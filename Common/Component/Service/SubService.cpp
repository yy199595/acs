//
// Created by yjz on 2022/1/22.
//
#include"SubService.h"
#include"Component/Redis/MainRedisComponent.h"
namespace Sentry
{
	bool SubService::LateAwake()
	{
		this->mRedisComponent = this->GetComponent<MainRedisComponent>();
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
			string channel = fmt::format("{0}.{1}", this->GetName(), func);
			if(this->mRedisComponent->Publish(channel, jsonWriter) ==0)
			{
				LOG_ERROR("publish [" << channel << "] error");
				return false;
			}
			return true;
		}
		return false;
	}

	bool SubService::Publish(const std::string& address, const std::string& func, Json::Writer& jsonWriter)
	{
		if (this->mRedisComponent != nullptr)
		{
			string channel = fmt::format("{0}.{1}", this->GetName(), func);
			jsonWriter.AddMember("func", channel);
			if (this->mRedisComponent->Publish(address, jsonWriter) == 0)
			{
				LOG_ERROR("publish [" << address
					<< "]  [" << channel << "] error");
				return false;
			}
			return true;
		}
		return false;
	}

	bool SubService::Invoke(const std::string& func, const Json::Reader & jsonReader)
	{
		LOG_CHECK_RET_FALSE(this->IsStartService());
		std::shared_ptr<SubMethod> subMethod = this->mServiceRegister->GetMethod(func);
		if(subMethod != nullptr)
		{
			subMethod->OnPublish(jsonReader);
			return true;
		}
		return false;
	}

	XCode SubService::Invoke(const std::string& func, const Json::Reader& jsonReader, Json::Writer& response)
	{
		if(!this->IsStartService())
		{
			return XCode::CallServiceNotFound;
		}
		std::shared_ptr<SubMethod> subMethod = this->mServiceRegister->GetMethod(func);
		if(subMethod == nullptr)
		{
			return XCode::CallFunctionNotExist;
		}
		if(subMethod != nullptr)
		{
			subMethod->OnPublish(jsonReader, response);
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
