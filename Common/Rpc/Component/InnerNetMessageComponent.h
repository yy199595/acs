#pragma once
#include"RpcTaskComponent.h"
#include"Client/Message.h"
#include"Async/RpcTaskSource.h"
#define MAX_HANDLER_MSG_COUNT 500 //每次循环最大处理数量
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
    class InnerNetMessageComponent : public RpcTaskComponent<long long, Rpc::Packet>, public IFrameUpdate
	{
	 public:
		InnerNetMessageComponent() = default;
    public:
        bool Ping(const std::string & address);
		XCode OnRequest(std::shared_ptr<Rpc::Packet> request);
        bool Send(const std::string & address, std::shared_ptr<Rpc::Packet> message);
        std::shared_ptr<Rpc::Packet> Call(const std::string & address, std::shared_ptr<Rpc::Packet> message);
    private:
		bool Awake() final;
		bool LateAwake() final;
		void OnFrameUpdate(float t) final;
        void Invoke(const RpcMethodConfig * config, std::shared_ptr<Rpc::Packet> message);
    private:
        std::string mFullName;
		class TaskComponent* mTaskComponent;
		class InnerNetComponent* mRpcClientComponent;
        std::queue<std::shared_ptr<Rpc::Packet>> mWaitMessages;
	};
}