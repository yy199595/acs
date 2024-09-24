//
// Created by 64658 on 2024/8/5.
//

#include "HttpApiComponent.h"
#include "HttpComponent.h"
#include "Http/Common/HttpResponse.h"
namespace acs
{
	HttpApiComponent::HttpApiComponent()
	{
		this->mHttp = nullptr;
	}

	bool HttpApiComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mHttp = this->GetComponent<HttpComponent>());
		return true;
	}

	std::unique_ptr<api::IPHomePlace> HttpApiComponent::GetHomePlace(const std::string& ip) const
	{
		std::string url = fmt::format("http://ip-api.com/json/{}?lang=zh-CN", ip);
		http::Response * response = this->mHttp->Get(url);
		if(response == nullptr || response->Code() != HttpStatus::OK)
		{
			return nullptr;
		}
		const http::JsonContent * jsonContent = response->To<http::JsonContent>();
		if(jsonContent == nullptr)
		{
			return nullptr;
		}
		std::unique_ptr<api::IPHomePlace> homePlace = std::make_unique<api::IPHomePlace>();
		{
			jsonContent->Get("as", homePlace->as);
			jsonContent->Get("org", homePlace->org);
			jsonContent->Get("isp", homePlace->isp);
			jsonContent->Get("lat", homePlace->lat);
			jsonContent->Get("lon", homePlace->lon);
			jsonContent->Get("city", homePlace->city);
			jsonContent->Get("query", homePlace->query);
			jsonContent->Get("region", homePlace->region);
			jsonContent->Get("status", homePlace->status);
			jsonContent->Get("country", homePlace->country);
			jsonContent->Get("regionName", homePlace->regionName);
			jsonContent->Get("countryCode", homePlace->countryCode);
		}
		return homePlace;
	}
}