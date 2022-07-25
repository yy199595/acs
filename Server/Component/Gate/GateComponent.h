
//
// Created by mac on 2021/11/28.
//

#ifndef GAMEKEEPER_PROTOPROXYCOMPONENT_H
#define GAMEKEEPER_PROTOPROXYCOMPONENT_H
#include"Component/Component.h"
#include"Async/RpcTask/RpcTaskSource.h"
#include"Task/RequestTaskQueueSource.h"

namespace Sentry
{
    class GateComponent;
    class ClientRpcTask : public IRpcTask<com::Rpc::Response>
    {
    public:
        ClientRpcTask(const c2s::Rpc::Request & request, GateComponent * component);

    public:
        int GetTimeout() final { return 0; }
        long long GetRpcId() final { return this->mTaskId; }
    private:
        void OnResponse(std::shared_ptr<com::Rpc::Response> response) final;
    private:
        long long mRpcId;
        long long mTaskId;
        std::string mAddress;
        GateComponent * mGateComponent;
    };
}

namespace Sentry
{
	class GateComponent final : public Component,
        public IClientRpc<c2s::Rpc::Request, com::Rpc::Response>
	{
	 public:
		GateComponent() = default;
		~GateComponent() final = default;
	 public:
		XCode OnRequest(std::shared_ptr<c2s::Rpc_Request> request) final;
		XCode OnResponse(const std::string & address, std::shared_ptr<com::Rpc::Response> response) final;
	private:
		bool LateAwake() final;
	 private:
		class TaskComponent * mTaskComponent;
		class TimerComponent * mTimerComponent;
		class GateClientComponent* mGateClientComponent;
        class ServiceRpcComponent * mServiceRpcComponent;
    };
}


#endif //GAMEKEEPER_PROTOPROXYCOMPONENT_H
