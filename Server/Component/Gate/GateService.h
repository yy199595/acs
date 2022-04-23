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
	 public:
		void GetAllAddress(std::vector<std::string> & gateAddress);
	private:
		XCode Ping(long long userId);
		XCode CallClient(long long userId, c2s::Rpc::Call & request);
		XCode BroadCast(const s2s::GateBroadCast::Request & request);
		XCode Allot(const s2s::AddressAllot::Request & request, s2s::AddressAllot::Response & response);
	 private:
		bool LateAwake() final;
		bool OnInitService(ServiceMethodRegister & methodRegister) final;
	 private:
		std::string mGateAddress;
		class UserSubService * mUserService;
		class TimerComponent* mTimerComponent;
		class GateComponent * mGateComponent;
		class GateClientComponent* mGateClientComponent;
	};

}

#endif //GAMEKEEPER_GATESERVICE_H
