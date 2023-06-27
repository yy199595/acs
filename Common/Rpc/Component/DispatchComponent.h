#pragma once
#include"RpcTaskComponent.h"
#include"Rpc/Client/Message.h"
#include"Rpc/Async/RpcTaskSource.h"
#include"Rpc/Config/MethodConfig.h"

namespace Tendo
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
	class DispatchComponent : public RpcTaskComponent<int, Msg::Packet>
	{
	 public:
		DispatchComponent();
    public:
		int OnMessage(const std::shared_ptr<Msg::Packet>& message);
    private:
		bool LateAwake() final;
		int OnPublishMessage(const std::shared_ptr<Msg::Packet>& message);
		int OnForwardMessage(const std::shared_ptr<Msg::Packet>& message);
		int OnRequestMessage(const std::shared_ptr<Msg::Packet> & message);
		int OnResponseMessage(const std::shared_ptr<Msg::Packet>& message);
		int OnBroadcastMessage(const std::shared_ptr<Msg::Packet>& message);
		void Invoke(const RpcMethodConfig * config, const std::shared_ptr<Msg::Packet>& message);
	private:
		void AddWaitCount(const std::string & name);
		void SubWaitCount(const std::string & name);
		bool EncodeJson(const std::shared_ptr<Msg::Packet> & message, std::string & json) const;
    private:
		unsigned int mWaitCount;
		class TimerComponent* mTimerComponent;
		class CoroutineComponent* mTaskComponent;
		class OuterNetComponent* mOuterComponent;
		class RouterComponent * mRouterComponent;
		std::unordered_map<std::string, int> mWaitCounts;
		std::unordered_map<std::string, int> mDoneCounts;
	};
}