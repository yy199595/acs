#pragma once

#include"Protocol/c2s.pb.h"
#include"Component/User/UserSyncComponent.h"
#include"Component/HttpService/HttpService.h"
#define USER_ID_START 7788
namespace Sentry
{

	class ServiceProxy;
	class MainRedisComponent;
	class MysqlProxyComponent;
	class HttpUserService : public HttpService, public IComplete
	{
	 public:
		HttpUserService() = default;
		~HttpUserService() final = default;

	 protected:
		void Awake() final;
		bool LateAwake() final;
		void OnAllServiceStart() final;
		bool OnInitService(HttpServiceRegister &serviceRegister) final;
	 private:
		XCode Login(const Json::Reader& request, Json::Writer& response);

		XCode Register(const Json::Reader& request, Json::Writer& response);

	 private:
		const std::string NewToken(const std::string& account);
	 private:
		class GateService * mGateService;
		UserSyncComponent * mUserSyncComponent;
		class MysqlProxyComponent * mMysqlComponent;
	};
}// namespace Sentry