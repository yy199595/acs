//
// Created by zmhy0073 on 2021/12/1.
//

#ifndef GAMEKEEPER_RPCPROXYTASK_H
#define GAMEKEEPER_RPCPROXYTASK_H
#include"RpcTask.h"

namespace GameKeeper
{
    class RpcComponent;
    class GateComponent;
    class RpcProxyTask : public RpcTaskBase
    {
    public:
        RpcProxyTask(int methodId);
        ~RpcProxyTask() final = default;

    public:
        void InitProxyTask(long long rpcId, long long sockId,
                           GateComponent * component, RpcComponent * rpcComponent );
    protected:
        void OnResponse(const com::Rpc_Response *response) final;
    private:
        long long mRpcId;
        long long mSockId;
        GateComponent * mProxyComponent;
    };
}


#endif //GAMEKEEPER_RPCPROXYTASK_H
