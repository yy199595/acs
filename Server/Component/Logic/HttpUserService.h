#pragma once

#include"Protocol/c2s.pb.h"
#include"Component/HttpService/HttpService.h"
#define USER_ID_START 7788
namespace Sentry
{

	class ServiceProxy;
	class RedisComponent;
	class MysqlProxyComponent;
	class HttpUserService : public HttpService
	{
	 public:
		HttpUserService() = default;
		~HttpUserService() final = default;

	 protected:
		void Awake() final;
		bool LateAwake() final;
		bool OnInitService(HttpServiceRegister &serviceRegister) final;
	 private:
		XCode Login(const Json::Reader& request, Json::Writer& response);

		XCode Register(const Json::Reader& request, Json::Writer& response);

	 private:
		const std::string NewToken(const std::string& account);
	 private:
		RedisComponent* mRedisComponent;
		MysqlProxyComponent* mMysqlComponent;
		std::shared_ptr<ServiceProxy> mGateService;
	};
}// namespace Sentry