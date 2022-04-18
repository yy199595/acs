//
// Created by zmhy0073 on 2021/12/1.
//

#ifndef GAMEKEEPER_GATESERVICE_H
#define GAMEKEEPER_GATESERVICE_H
#include"Component/RpcService/LocalServerRpc.h"
namespace Sentry
{
	class GateService final : public LocalServerRpc
	{
	 public:
		GateService() = default;
		~GateService() final = default;
	 private:
		XCode Ping();
		XCode Login(const std::string & address,const c2s::GateLogin::Request& request);
		XCode Allot(const s2s::AddressAllot::Request & request, s2s::AddressAllot::Response & response);
	 private:
		bool LateAwake() final;
		void OnTokenTimeout(const std::string& token);
		bool OnInitService(ServiceMethodRegister & methodRegister) final;
	 private:
		std::string mGateAddress;
		class UserSubService * mUserService;
		class TimerComponent* mTimerComponent;
		class GateClientComponent* mGateComponent;
		std::unordered_map<std::string, long long> mTokenMap;
	};

}

#endif //GAMEKEEPER_GATESERVICE_H
