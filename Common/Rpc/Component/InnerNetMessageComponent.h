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
		InnerNetMessageComponent() = default;
    public:
        bool Ping(const std::string & address);
		int OnMessage(std::shared_ptr<Rpc::Packet> message);
        bool Send(const std::string & address, std::shared_ptr<Rpc::Packet> message);
		bool Send(const std::string & address, std::shared_ptr<Rpc::Packet> message, int & id);
		std::shared_ptr<Rpc::Packet> Call(const std::string & address, std::shared_ptr<Rpc::Packet> message);
    private:
		bool Awake() final;
		bool LateAwake() final;
		int HandlerForward(std::shared_ptr<Rpc::Packet> message);
		int HandlerRequest(std::shared_ptr<Rpc::Packet> message);
		int HandlerResponse(std::shared_ptr<Rpc::Packet> message);
		int HandlerBroadcast(std::shared_ptr<Rpc::Packet> message);
		void OnTaskComplete(int key) final { this->mNumberPool.Push(key); }
		void Send(const std::string& address, int code, std::shared_ptr<Rpc::Packet> pack);
        void Invoke(const RpcMethodConfig * config, std::shared_ptr<Rpc::Packet> message);
    private:
        std::string mFullName;
		class AsyncMgrComponent* mTaskComponent;
		class TimerComponent* mTimerComponent;
		Util::NumberBuilder<int, 1> mNumberPool;
		class OuterNetComponent* mOuterComponent;
		class InnerNetComponent* mInnerComponent;
	};
}