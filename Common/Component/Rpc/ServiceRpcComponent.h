#pragma once

#include"Other/MultiThreadQueue.h"
#include"Async/RpcTask/RpcTaskSource.h"
#include"Component/Rpc/RpcTaskComponent.h"
namespace Sentry
{
	class ServiceComponent;

	class LocalLuaService;

	class ServiceMethod;

#ifdef __DEBUG__
	struct RpcTaskInfo
	{
		int MethodId;
		long long Time;
	};
#endif
    class ServiceRpcComponent : public RpcTaskComponent<com::Rpc::Response>
	{
	 public:
		ServiceRpcComponent() = default;
		~ServiceRpcComponent() final = default;
	 protected:
		void Awake() final;
		bool LateAwake() final;
	 public:
		XCode OnRequest(std::shared_ptr<com::Rpc_Request> request);
    private:
		void OnTaskTimeout(long long rpcId);
	 private:
		std::string mTempMethod;
		std::string mTempService;
		class TaskComponent* mTaskComponent;
		class TimerComponent* mTimerComponent;
		class RpcClientComponent* mRpcClientComponent;
	};
}