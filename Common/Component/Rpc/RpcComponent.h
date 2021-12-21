#pragma once

#include<Protocol/s2s.pb.h>
#include<Component/Component.h>
#include<Other/MultiThreadQueue.h>
#include<Pool/ObjectPool.h>
namespace GameKeeper
{
    class ServiceComponent;

    class LuaServiceComponent;

	class ServiceMethod;

    class JsonServiceMethod;

    class RpcTaskBase;
    class RpcComponent : public Component,
                         public IProtoRpc<com::Rpc_Request, com::Rpc_Response>
    {
    public:
		RpcComponent() = default;
        ~RpcComponent() final = default;

    public:
        unsigned int AddRpcTask(std::shared_ptr<RpcTaskBase> task);
        std::shared_ptr<RpcTaskBase> GetRpcTask(long long rpcId) const;
    protected:
        bool Awake() final;
        bool LateAwake() final;
    public:
        XCode OnRequest(const com::Rpc_Request * message) final;
        XCode OnResponse(const com::Rpc_Response *response) final;
        virtual int GetPriority() const { return 500; }
	private:
        void OnTaskTimeout(long long rpcId);
    private:
        int mTick;
        int mNodeId;
        long long mLastTime;
        class TaskComponent *mCorComponent;
        class TimerComponent * mTimerComponent;
        class RpcClientComponent *mRpcClientComponent;
        class RpcConfigComponent * mPpcConfigComponent;
        std::unordered_map<long long, std::shared_ptr<RpcTaskBase>> mRpcTasks;
    };
}