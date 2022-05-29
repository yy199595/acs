//
// Created by yjz on 2022/1/23.
//
#include"LoclHttpService.h"
#include"App/App.h"
#include"spdlog/fmt/fmt.h"
#include"Network/Http/HttpAsyncRequest.h"
#include"Component/Http/HttpComponent.h"
namespace Sentry
{
	XCode LoclHttpService::Invoke(const std::string& name, std::shared_ptr<Json::Reader> request,
			std::shared_ptr<Json::Writer> response)
	{
		std::shared_ptr<HttpServiceMethod> method = this->mServiceRegister->GetMethod(name);
		if(method == nullptr)
		{
			return XCode::CallServiceNotFound;
		}
		return method->Invoke(request, response);
	}
	bool LoclHttpService::StartService()
	{
		this->mServiceRegister = std::make_shared<HttpServiceRegister>(this);
		return this->OnStartService(*this->mServiceRegister);
	}

	bool LoclHttpService::CloseService()
	{
		return false;
	}
}