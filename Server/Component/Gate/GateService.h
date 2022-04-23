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
		XCode Ping(long long userId);
	private:
		XCode Allot(const s2s::AddressAllot::Request & request, s2s::AddressAllot::Response & response);
	 private:
		bool LateAwake() final;
		bool OnInitService(ServiceMethodRegister & methodRegister) final;
	 public:
		void BroadCast(const std::string & func);
		void BroadCast(const std::string & func, const Message & message);
	 private:
		std::string mGateAddress;
		class UserSubService * mUserService;
		class TimerComponent* mTimerComponent;
		class GateComponent * mGateComponent;
		class GateClientComponent* mGateClientComponent;
	};

}

#endif //GAMEKEEPER_GATESERVICE_H
