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
		XCode Ping(const std::string & address);
		XCode QueryAddress(com::Type::String & response);
		XCode CallClient(long long userId, c2s::Rpc::Call & request);
		XCode BroadCast(const s2s::GateBroadCast::Request & request);
		XCode Auth(const std::string & address, const c2s::GateAuth::Request & request);
	 private:
		bool LateAwake() final;
		bool OnInitEvent(ServiceEventRegister &methodRegister) final;
		bool OnStartService(ServiceMethodRegister & methodRegister) final;
	 private:
		std::string mAddress;
		class GateComponent * mGateComponent;
		class TimerComponent* mTimerComponent;
		class UserSyncComponent * mSyncComponent;
		class GateClientComponent* mGateClientComponent;
	};

}

#endif //GAMEKEEPER_GATESERVICE_H
