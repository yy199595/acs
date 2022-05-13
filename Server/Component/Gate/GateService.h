//
// Created by zmhy0073 on 2021/12/1.
//

#ifndef GAMEKEEPER_GATESERVICE_H
#define GAMEKEEPER_GATESERVICE_H
#include"Component/RpcService/LocalServiceComponent.h"
namespace Sentry
{
	class GateService final : public LocalServiceComponent
	{
	 public:
		GateService() = default;
		~GateService() final = default;
	private:
		XCode Ping(long long userId);
		XCode Auth(const c2s::GateAuth::Request & request);
		XCode CallClient(long long userId, c2s::Rpc::Call & request);
		XCode BroadCast(const s2s::GateBroadCast::Request & request);
		XCode Allot(const s2s::Allot::Request & request, s2s::Allot::Response & response);
	 private:
		bool LateAwake() final;
		bool OnInitEvent(ServiceEventRegister &methodRegister) final;
		bool OnInitService(ServiceMethodRegister & methodRegister) final;
	 private:
		std::string mGateAddress;
		class TimerComponent* mTimerComponent;
		class GateComponent * mGateComponent;
		class UserInfoSyncService * mUserService;
		class GateClientComponent* mGateClientComponent;
	};

}

#endif //GAMEKEEPER_GATESERVICE_H
