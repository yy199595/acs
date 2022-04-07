//
// Created by zmhy0073 on 2021/12/1.
//

#ifndef GAMEKEEPER_GATESERVICE_H
#define GAMEKEEPER_GATESERVICE_H
#include"Component/RpcService/LocalServerRpc.h"
namespace Sentry
{
	class GateService : public LocalServerRpc
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
	 private:
		void OnTokenTimeout(const std::string& token);
	 private:
		class TimerComponent* mTimerComponent;
		class GateClientComponent* mGateComponent;
		class EntityMgrComponent* mEntityComponent;
		std::unordered_map<std::string, long long> mTokenMap;
	};

}

#endif //GAMEKEEPER_GATESERVICE_H
