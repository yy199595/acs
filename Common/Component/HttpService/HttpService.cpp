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

	XCode HttpService::Invoke(const std::string& name, std::shared_ptr<Json::Reader> request,
			std::shared_ptr<Json::Writer> response)
	{
		std::shared_ptr<HttpServiceMethod> method = this->mServiceRegister->GetMethod(name);
		if(method == nullptr)
		{
			return XCode::CallServiceNotFound;
		}
		return method->Invoke(request, response);
	}
	bool HttpService::StartService()
	{
		this->mServiceRegister = std::make_shared<HttpServiceRegister>(this);
		return this->OnStartService(*this->mServiceRegister);
	}

	XCode HttpService::Get(const std::string& path, std::shared_ptr<Json::Reader> response)
	{
		if(this->mUrl.empty())
		{
			return XCode::NetWorkError;
		}
		const std::string url = fmt::format("%s%s", this->mUrl, path);
		std::shared_ptr<HttpAsyncResponse> httpResponse = this->mHttpComponent->Get(url);
		if(httpResponse == nullptr)
		{
			return XCode::NetWorkError;
		}
#ifdef __HTTP_DEBUG_LOG__
		LOG_INFO("[GET] " << url << " response = " << httpResponse->GetContent());
#endif
		if(!response->ParseJson(httpResponse->GetContent()))
		{
			return XCode::ParseJsonFailure;
		}
		XCode code = XCode::Failure;
		return response->GetMember("code", code) ? code : XCode::Failure;
	}

	XCode HttpService::Post(const std::string& path, Json::Writer& request, std::shared_ptr<Json::Reader> response)
	{
		if(this->mUrl.empty())
		{
			return XCode::NetWorkError;
		}
		std::string json;
		if(!request.WriterStream(json))
		{
			return XCode::ParseJsonFailure;
		}
		const std::string url = fmt::format("{0}{1}", this->mUrl, path);
		std::shared_ptr<HttpAsyncResponse> httpResponse = this->mHttpComponent->Post(url, json);
		if(httpResponse == nullptr)
		{
			return XCode::NetWorkError;
		}
		LOG_INFO("[POST] " << url <<" request = " << json << " response = " << httpResponse->GetContent());
		if(!response->ParseJson(httpResponse->GetContent()))
		{
			return XCode::ParseJsonFailure;
		}
		XCode code = XCode::Failure;
		return response->GetMember("code", code) ? code : XCode::Failure;
	}

	bool HttpService::CloseService()
	{
		return false;
	}
}