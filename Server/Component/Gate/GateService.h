//
// Created by zmhy0073 on 2021/12/1.
//

#ifndef GAMEKEEPER_GATESERVICE_H
#define GAMEKEEPER_GATESERVICE_H
#include"Component/Service/RpcService.h"
namespace Sentry
{
	class GateService : public RpcService
	{
	 public:
		GateService() = default;
		~GateService() final = default;
	 protected:
		bool Awake() final;
		bool LateAwake() final;
	 private:
		XCode Ping();
		XCode Login(const c2s::GateLogin::Request& request);
		XCode Allot(const s2s::AddToGate_Request& request, s2s::AddToGate_Response& response);

	 private:
		void OnTokenTimeout(const std::string& token);
	 private:
		class TimerComponent* mTimerComponent;
		class GateClientComponent* mGateComponent;
		class EntityMgrComponent* mEntityComponent;
		const class NetworkListener* mGateListener;
		std::unordered_map<std::string, long long> mTokenMap;
	};

}

#endif //GAMEKEEPER_GATESERVICE_H
