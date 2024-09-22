//
// Created by 64658 on 2024/8/5.
//

#ifndef APP_HTTPAPICOMPONENT_H
#define APP_HTTPAPICOMPONENT_H
#include "Entity/Component/Component.h"

namespace api
{
	struct IPHomePlace
	{
		std::string status;
		std::string country;
		std::string countryCode;
		std::string region;
		std::string regionName;
		std::string city;
		double lat = 0;
		double lon = 0;
		std::string timezone;
		std::string isp;
		std::string org;
		std::string as;
		std::string query;
	};
}

namespace acs
{
	class HttpApiComponent : public Component, public IComplete
	{
	public:
		HttpApiComponent();
		~HttpApiComponent() final = default;
	public:
		std::unique_ptr<api::IPHomePlace> GetHomePlace(const std::string & ip) const;
	private:
		bool LateAwake() final;
		void Complete() final;
	private:
		class HttpComponent * mHttp;
	};
}

#endif //APP_HTTPAPICOMPONENT_H
