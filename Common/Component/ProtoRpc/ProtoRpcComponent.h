#pragma once

#include<Protocol/s2s.pb.h>
#include<Component/Component.h>
#include<Other/MultiThreadQueue.h>
#include<Pool/ObjectPool.h>
namespace GameKeeper
{
    class ServiceComponent;

    class ProtoServiceComponent;
    
    class LuaServiceComponent;

	class ServiceMethod;

    class JsonServiceMethod;

    class ProtoRpcTask;
    class ProtoRpcComponent : public Component,
            public IProtoRpc<com::Rpc_Request, com::Rpc_Response>
    {
    public:
		ProtoRpcComponent() = default;
        ~ProtoRpcComponent() final = default;

    public:
        long long GetRpcTaskId();
        unsigned int AddRpcTask(std::shared_ptr<ProtoRpcTask> task);
        std::shared_ptr<ProtoRpcTask> GetRpcTask(long long rpcId) const;
    protected:
        bool Awake() final;
        bool LateAwake() final;
    public:
        bool OnRequest(const com::Rpc_Request * message) final;
        bool OnResponse(const com::Rpc_Response *response) final;
        virtual int GetPriority() const { return 500; }
	private:
        void OnTaskTimeout(long long rpcId);
        void AwaitInvoke(ServiceMethod * method, const com::Rpc_Request * request);
    private:
        int mTick;
        int mNodeId;
        long long mLastTime;
        class TimerComponent * mTimerComponent;
        class CoroutineComponent *mCorComponent;
        class RpcConfigComponent * mPpcConfigComponent;
        class ProtoRpcClientComponent *mRpcClientComponent;
        std::unordered_map<long long, std::shared_ptr<ProtoRpcTask>> mRpcTasks;
    };
}