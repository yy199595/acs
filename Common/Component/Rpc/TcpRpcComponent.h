#pragma once
#include"Async/RpcTask/RpcTaskSource.h"
#include"Component/Rpc/RpcTaskComponent.h"
namespace Sentry
{
	class Service;

	class LuaService;

	class ServiceMethod;

#ifdef __DEBUG__
	struct RpcTaskInfo
	{
		int MethodId;
		long long Time;
	};
#endif
    class TcpRpcComponent : public RpcTaskComponent<com::rpc::response>
	{
	 public:
		TcpRpcComponent() = default;
		~TcpRpcComponent() final = default;
	 protected:
		void Awake() final;
		bool LateAwake() final;
	 public:
		XCode OnRequest(std::shared_ptr<com::rpc::request> request);
    private:
		void OnTaskTimeout(long long rpcId);
	 private:
		std::string mFullName;
		std::string mTempMethod;
		std::string mTempService;
		class TaskComponent* mTaskComponent;
		class TimerComponent* mTimerComponent;
		class RpcClientComponent* mRpcClientComponent;
	};
}