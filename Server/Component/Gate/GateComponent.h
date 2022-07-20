
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
    class GateClientComponent;
    class ClientRpcTask : public IRpcTask<com::Rpc::Response>
    {
    public:
        ClientRpcTask(const c2s::Rpc::Request & request, GateClientComponent * component);

    public:
        int GetTimeout() final { return 0; }
        long long GetRpcId() final { return this->mTaskId; }
    private:
        void OnResponse(std::shared_ptr<Rpc_Response> response) final;
    private:
        long long mTaskId;
        std::string mAddress;
        GateClientComponent * mClientComponent;
        std::shared_ptr<c2s::Rpc::Response> mResponse;
    };
}

namespace Sentry
{
	class GateComponent final : public Component,
        public IClientRpc<c2s::Rpc::Request, c2s::Rpc::Response>
	{
	 public:
		GateComponent() = default;
		~GateComponent() final = default;
	 public:
		XCode OnRequest(std::shared_ptr<c2s::Rpc_Request> request) final;
		XCode OnResponse(const std::string & address, std::shared_ptr<c2s::Rpc_Response> response) final;
	private:
		bool LateAwake() final;
		void OnUserRequest(const RpcInterfaceConfig * config, std::shared_ptr<com::Rpc::Request> request);
		XCode HandlerRequest(const RpcInterfaceConfig * config, std::shared_ptr<com::Rpc::Request> request, std::shared_ptr<c2s::Rpc::Response> response);
	 private:
		class TaskComponent * mTaskComponent;
		class TimerComponent * mTimerComponent;
		class MessageComponent * mMsgComponent;
		class UserSyncComponent * mUserSyncComponent;
		class GateClientComponent* mGateClientComponent;
    };
}


#endif //GAMEKEEPER_PROTOPROXYCOMPONENT_H
