
//
// Created by mac on 2021/11/28.
//

#ifndef GAMEKEEPER_PROTOPROXYCOMPONENT_H
#define GAMEKEEPER_PROTOPROXYCOMPONENT_H
#include"Client/Message.h"
#include"Component/Component.h"
#include"Async/RpcTaskSource.h"

namespace Sentry
{
    class OuterNetMessageComponent;
    class ClientRpcTask : public IRpcTask<Rpc::Data>
    {
    public:
        ClientRpcTask(Rpc::Data & request, OuterNetMessageComponent * component, int ms);

    public:
        long long GetRpcId() final { return this->mTaskId; }
    private:
        void OnTimeout() final;
        void OnResponse(std::shared_ptr<Rpc::Data> response) final;
    private:
        long long mRpcId;
        long long mTaskId;
        std::string mAddress;
        OuterNetMessageComponent * mGateComponent;
    };
}

namespace Sentry
{
	class OuterNetMessageComponent final : public Component
	{
	 public:
		OuterNetMessageComponent() = default;
		~OuterNetMessageComponent() final = default;
	 public:       
        XCode OnRequest(const std::string & address, std::shared_ptr<Rpc::Data> message);
        XCode OnResponse(const std::string & address, std::shared_ptr<Rpc::Data> message);
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
