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

#ifdef __DEBUG__
    struct RpcTaskInfo
    {
        int MethodId;
        long long Time;
    };
#endif

    class IRpcTask;
    class RpcComponent : public Component,
                         public IProtoRpc<com::Rpc_Request, com::Rpc_Response>
    {
    public:
		RpcComponent() = default;
        ~RpcComponent() final = default;

    public:
        void AddRpcTask(std::shared_ptr<IRpcTask> task);
#ifdef __DEBUG__
        void AddRpcInfo(long long rpcId, int methodId);
        bool GetRpcInfo(long long rpcId, int & methodId, long long & time) const;
#endif
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
        int mNodeId;
        class TaskComponent *mCorComponent;
        class TimerComponent * mTimerComponent;
        class RpcClientComponent *mRpcClientComponent;
        class RpcConfigComponent * mPpcConfigComponent;
        std::unordered_map<long long, RpcTaskInfo> mRpcInfoMap;
        std::unordered_map<long long, std::shared_ptr<IRpcTask>> mRpcTasks;
    };
}