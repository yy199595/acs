//
// Created by zmhy0073 on 2021/12/1.
//

#ifndef GAMEKEEPER_PROTOPROXYTASK_H
#define GAMEKEEPER_PROTOPROXYTASK_H
#include"ProtoRpcTask.h"

namespace GameKeeper
{
    class ProtoRpcComponent;
    class ProtoGateComponent;
    class ProtoProxyTask : public ProtoRpcTask
    {
    public:
        ProtoProxyTask(int methodId, long long rpcId);
        ~ProtoProxyTask() final = default;

    public:
        void InitProxyTask(long long rpcId, long long sockId,
                           ProtoGateComponent * component, ProtoRpcComponent * rpcComponent );
    protected:
        void OnResponse(const com::Rpc_Response *response) final;
    private:
        long long mClientRpcId;
        long long mClientSockId;
        ProtoGateComponent * mProxyComponent;
    };
}


#endif //GAMEKEEPER_PROTOPROXYTASK_H
