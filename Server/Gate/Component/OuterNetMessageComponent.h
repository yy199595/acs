
//
// Created by mac on 2021/11/28.
//

#ifndef GAMEKEEPER_PROTOPROXYCOMPONENT_H
#define GAMEKEEPER_PROTOPROXYCOMPONENT_H
#include"Component/Component.h"
#include"Async/RpcTaskSource.h"

namespace Sentry
{
    class OuterNetMessageComponent;
    class ClientRpcTask : public IRpcTask<com::rpc::response>
    {
    public:
        ClientRpcTask(const c2s::rpc::request & request, OuterNetMessageComponent * component, int ms);

    public:
        long long GetRpcId() final { return this->mTaskId; }
    private:
        void OnTimeout() final;
        void OnResponse(std::shared_ptr<com::rpc::response> response) final;
    private:
        long long mRpcId;
        long long mTaskId;
        std::string mAddress;
        OuterNetMessageComponent * mGateComponent;
    };
}

namespace Sentry
{
	class OuterNetMessageComponent final : public Component,
                                           public IClientRpc<c2s::rpc::request, com::rpc::response>
	{
	 public:
		OuterNetMessageComponent() = default;
		~OuterNetMessageComponent() final = default;
	 public:       
        XCode OnRequest(std::shared_ptr<c2s::rpc::request> request) final;
        XCode OnResponse(const std::string & address, std::shared_ptr<com::rpc::response> response) final;
    private:
		bool LateAwake() final;
	 private:
		class TaskComponent * mTaskComponent;
		class TimerComponent * mTimerComponent;
        class OuterNetComponent* mOutNetComponent;
        class InnerNetMessageComponent * mInnerMessageComponent;
    };
}


#endif //GAMEKEEPER_PROTOPROXYCOMPONENT_H