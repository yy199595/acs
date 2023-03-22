#pragma once
#include"RpcTaskComponent.h"
#include"Client/Message.h"
#include"Async/RpcTaskSource.h"
#define MAX_HANDLER_MSG_COUNT 500 //每次循环最大处理数量
namespace Sentry
{
	class RpcService;

	class LuaPhysicalService;

	class ServiceMethod;

#ifdef __DEBUG__
	struct RpcTaskInfo
	{
		int MethodId;
		long long Time;
	};
#endif
    class InnerNetMessageComponent : public RpcTaskComponent<int, Rpc::Packet>
	{
	 public:
		InnerNetMessageComponent();
    public:
        bool Ping(const std::string & address);
		int OnMessage(const std::shared_ptr<Rpc::Packet>& message);
		unsigned int WaitCount() const { return this->mWaitCount; }
        bool Send(const std::string & address, const std::shared_ptr<Rpc::Packet>& message);
		bool Send(const std::string & address, const std::shared_ptr<Rpc::Packet>& message, int & id);
		std::shared_ptr<Rpc::Packet> Call(const std::string & address, const std::shared_ptr<Rpc::Packet>& message);
    private:
		bool LateAwake() final;
		int HandlerForward(const std::shared_ptr<Rpc::Packet>& message);
		int HandlerRequest(std::shared_ptr<Rpc::Packet> message);
		int HandlerResponse(const std::shared_ptr<Rpc::Packet>& message);
		int HandlerBroadcast(const std::shared_ptr<Rpc::Packet>& message);
		void OnTaskComplete(int key) final { this->mNumberPool.Push(key); }
		void Send(const std::string& address, int code, const std::shared_ptr<Rpc::Packet>& pack);
        void Invoke(const RpcMethodConfig * config, const std::shared_ptr<Rpc::Packet>& message);
    private:
		unsigned int mWaitCount;
		class AsyncMgrComponent* mTaskComponent;
		class TimerComponent* mTimerComponent;
		Util::NumberBuilder<int, 1> mNumberPool;
		class OuterNetComponent* mOuterComponent;
		class InnerNetComponent* mInnerComponent;
	};
}