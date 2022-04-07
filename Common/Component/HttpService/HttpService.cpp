//
// Created by yjz on 2022/1/23.
//
#include"HttpService.h"

namespace Sentry
{
	std::shared_ptr<Json::Writer> HttpService::Invoke(
		const std::string& name, std::shared_ptr<Json::Reader> request)
	{
		std::shared_ptr<Json::Writer> response(new Json::Writer());
		auto iter = this->mHttpMethodMap.find(name);
		if (iter == this->mHttpMethodMap.end())
		{
			response->AddMember("code", (int)XCode::CallServiceNotFound);
			return response;
		}
		try
		{
			auto httpServiceMethod = iter->second;
			XCode code = httpServiceMethod->Invoke(request, response);
			response->AddMember("code", (int)code);
		}
		catch (std::logic_error& logic_error)
		{
			response->AddMember("error", logic_error.what());
			response->AddMember("code", (int)XCode::ThrowError);
		}
		return response;
	}
}