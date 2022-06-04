//
// Created by yjz on 2022/1/23.
//
#include"LocalHttpService.h"
#include"App/App.h"
#include"spdlog/fmt/fmt.h"
#include"Network/Http/HttpAsyncRequest.h"
#include"Component/Http/HttpComponent.h"
namespace Sentry
{
	LocalHttpService::LocalHttpService()
	{
		this->mConfig = nullptr;
	}
	XCode LocalHttpService::Invoke(const std::string& name,
		std::shared_ptr<HttpHandlerRequest> request, std::shared_ptr<HttpHandlerResponse> response)
	{
		std::shared_ptr<HttpServiceMethod> method = this->mServiceRegister->GetMethod(name);
		if(method == nullptr)
		{
			return XCode::CallServiceNotFound;
		}
		return method->Invoke(*request, *response);
	}
	bool LocalHttpService::StartService()
	{
		this->mServiceRegister = std::make_shared<HttpServiceRegister>(this);
		return this->OnStartService(*this->mServiceRegister);
	}

	bool LocalHttpService::CloseService()
	{
		return false;
	}
	bool LocalHttpService::LoadConfig(const rapidjson::Value& json)
	{
		if(this->mConfig == nullptr)
		{
			this->mConfig = new HttpServiceConfig(this->GetName());
		}
		return this->mConfig->OnLoadConfig(json);
	}
}