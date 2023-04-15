#pragma once
#include"RpcTaskComponent.h"
#include"Rpc/Client/Message.h"
#include"Rpc/Async/RpcTaskSource.h"
#include"Server/Config/MethodConfig.h"

namespace Tendo
{
	class RpcService;

	class LuaPhysicalRpcService;

	class ServiceMethod;

#ifdef __DEBUG__
	struct RpcTaskInfo
	{
		int MethodId;
		long long Time;
	};
#endif
    class DispatchMessageComponent : public RpcTaskComponent<int, Rpc::Packet>
	{
	 public:
		DispatchMessageComponent();
    public:
		int OnMessage(const std::shared_ptr<Rpc::Packet>& message);
		unsigned int WaitCount() const { return this->mWaitCount; }
    private:
		bool LateAwake() final;
		int HandlerForward(const std::shared_ptr<Rpc::Packet>& message);
		int HandlerRequest(const std::shared_ptr<Rpc::Packet> & message);
		int HandlerResponse(const std::shared_ptr<Rpc::Packet>& message);
		int HandlerBroadcast(const std::shared_ptr<Rpc::Packet>& message);
		void Invoke(const RpcMethodConfig * config, const std::shared_ptr<Rpc::Packet>& message);
    private:
		unsigned int mWaitCount;
		class InnerRpcComponent * mNetComponent;
		class AsyncMgrComponent* mTaskComponent;
		class TimerComponent* mTimerComponent;
		class OuterNetComponent* mOuterComponent;
	};
}