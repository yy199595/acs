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
    class InnerNetMessageComponent : public RpcTaskComponent<com::rpc::response>
	{
	 public:
		InnerNetMessageComponent() = default;
		~InnerNetMessageComponent() final = default;
	 protected:
		void Awake() final;
		bool LateAwake() final;
        void OnTaskTimeout(long long rpcId);
    public:
        bool OnProtoRequest(const std::string & address, const char * data, int len);
        bool OnProtoResponse(const std::string & address, const char * data, int len);

    private:
        XCode OnRequest(std::shared_ptr<com::rpc::request> request);
    private:
        std::string mMethod;
        std::string mService;
		std::string mFullName;
		class TaskComponent* mTaskComponent;
		class TimerComponent* mTimerComponent;
		class InnerNetComponent* mRpcClientComponent;
	};
}