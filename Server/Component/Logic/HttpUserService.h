#pragma once

#include"Protocol/c2s.pb.h"
#include"Component/User/UserSyncComponent.h"
#include"Component/HttpService/LocalHttpService.h"
#define USER_ID_START 7788
namespace Sentry
{
	class MainRedisComponent;
	class MysqlAgentComponent;
	class HttpUserService : public LocalHttpService, public IComplete
	{
	 public:
		HttpUserService() = default;
		~HttpUserService() final = default;

	 protected:
		void Awake() final;
		bool LateAwake() final;
		void OnAllServiceStart() final;
		bool OnStartService(HttpServiceRegister &serviceRegister) final;
	 private:
		XCode Login(const HttpHandlerRequest& request, HttpHandlerResponse& response);

		XCode Register(const HttpHandlerRequest& request, HttpHandlerResponse& response);

	 private:
		void NewToken(const std::string& account, std::string & token);
	 private:
		class GateService * mGateService;
		UserSyncComponent * mUserSyncComponent;
		class MysqlAgentComponent * mMysqlComponent;
	};
}// namespace Sentry