#pragma once
#include"RpcTaskComponent.h"
#include"Client/Message.h"
#include"Async/RpcTaskSource.h"
#define MAX_HANDLER_MSG_COUNT 100 //每次循环最大处理数量
namespace Sentry
{
	class RpcService;

	class LuaRpcService;

	class ServiceMethod;

#ifdef __DEBUG__
	struct RpcTaskInfo
	{
		int MethodId;
		long long Time;
	};
#endif
    class InnerNetMessageComponent : public RpcTaskComponent<Rpc::Data>, public ISystemUpdate
	{
	 public:
		InnerNetMessageComponent() = default;
		~InnerNetMessageComponent() final = default;
	 protected:
		bool Awake() final;
		bool LateAwake() final;
        void OnSystemUpdate() final;
        void OnTaskTimeout(long long rpcId);
    public:
		XCode OnRequest(std::shared_ptr<Rpc::Data> request);
    public:
        bool Send(const std::string & address, std::shared_ptr<Rpc::Data> message);
        std::shared_ptr<Rpc::Data> Call(const std::string & address, std::shared_ptr<Rpc::Data> message);
    private:
        void Invoke(const RpcMethodConfig * config, std::shared_ptr<Rpc::Data> message);
    private:
		std::string mFullName;
		class TaskComponent* mTaskComponent;
		class TimerComponent* mTimerComponent;
		class InnerNetComponent* mRpcClientComponent;
        std::queue<std::shared_ptr<Rpc::Data>> mWaitMessages;
	};
}