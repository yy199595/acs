//
// Created by zmhy0073 on 2021/12/1.
//

#include"RpcProxyTask.h"
#include"ServerRpc/RpcComponent.h"
#include"Component/GateComponent.h"
namespace GameKeeper
{
    RpcProxyTask::RpcProxyTask(int methodId)
        : RpcTaskBase(methodId)
    {
        this->mClientRpcId = 0;
        this->mClientSockId = 0;
        this->mProxyComponent = nullptr;
    }

    void RpcProxyTask::InitProxyTask(long long rpcId, long long sockId, GateComponent *component,
                                     RpcComponent *rpcComponent)
    {
        this->mClientRpcId = rpcId;
        this->mClientSockId = sockId;
        this->mProxyComponent = component;
        rpcComponent->AddRpcTask(this->shared_from_this());
    }

    void RpcProxyTask::OnResponse(const com::Rpc_Response *response)
    {
        auto responseMessage = new c2s::Rpc_Response();

        responseMessage->set_code(response->code());
        responseMessage->set_rpcid(this->mClientRpcId);
        if (response->has_data())
        {
            responseMessage->mutable_data()->CopyFrom(response->data());
        }
        XCode code = this->mProxyComponent->OnResponse(this->mClientRpcId, responseMessage);
        if(code != XCode::Successful)
        {
            delete responseMessage;
        }
    }

}