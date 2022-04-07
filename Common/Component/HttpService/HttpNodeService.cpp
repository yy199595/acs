//
// Created by yjz on 2022/1/26.
//

#include"HttpNodeService.h"
#include"App/App.h"
namespace Sentry
{
	bool HttpNodeService::Awake()
	{
		BIND_HTTP_FUNCTION(HttpNodeService::Push);
		return true;
	}

	XCode HttpNodeService::Push(const Json::Reader& jsonReader, Json::Writer& response)
	{
		std::string address;
		std::vector<std::string> services;
		LOGIC_THROW_ERROR(jsonReader.GetMember("rpc", "address", address));
		LOGIC_THROW_ERROR(jsonReader.GetMember("rpc", "service", services));
		for (const std::string& service : services)
		{
			//TODO
//			auto serviceProxy = this->mServiceComponent->GetServiceProxy(service);
//			LOGIC_THROW_ERROR(serviceProxy);
//			serviceProxy->AddAddress(address);
		}
		return XCode::Successful;
	}

	bool HttpNodeService::LateAwake()
	{
		return true;
	}
}