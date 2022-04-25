//
// Created by yjz on 2022/1/23.
//
#include"HttpService.h"
#include"App/App.h"
#include"spdlog/fmt/fmt.h"
#include"Network/Http/HttpAsyncRequest.h"
#include"Component/Http/HttpComponent.h"
namespace Sentry
{
	bool HttpService::LateAwake()
	{
		this->mHttpComponent = this->GetComponent<HttpComponent>();
		return true;
	}

	void HttpService::OnAddAddress(const string& address)
	{
		this->mUrl = fmt::format("http://{0}/", address);
		LOG_INFO("add new http url " << this->mUrl);
	}

	XCode HttpService::Invoke(
		const std::string& name, std::shared_ptr<Json::Reader> request, std::shared_ptr<Json::Writer> response)
	{
		std::shared_ptr<HttpServiceMethod> method = this->mServiceRegister->GetMethod(name);
		if(method == nullptr)
		{
			return XCode::CallServiceNotFound;
		}
		return method->Invoke(request, response);
	}
	bool HttpService::LoadService()
	{
		this->mServiceRegister = std::make_shared<HttpServiceRegister>(this);
		return this->OnInitService(*this->mServiceRegister);
	}
}