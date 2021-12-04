//
// Created by zmhy0073 on 2021/12/1.
//

#include"ProtoProxyTask.h"
#include"ProtoRpc/ProtoRpcComponent.h"
#include"ProxyRpc/ProtoProxyComponent.h"
namespace GameKeeper
{
    ProtoProxyTask::ProtoProxyTask(int methodId, long long int rpcId)
        : ProtoRpcTask(methodId, rpcId)
    {
        this->mTimerId = 0;
        this->mClientRpcId = 0;
        this->mClientSockId = 0;
        this->mProxyComponent = nullptr;
    }

    void ProtoProxyTask::InitProxyTask(long long rpcId, long long sockId, ProtoProxyComponent *component,
                                       ProtoRpcComponent *rpcComponent)
    {
        this->mClientRpcId = rpcId;
        this->mClientSockId = sockId;
        this->mProxyComponent = component;
        this->mTimerId = rpcComponent->AddRpcTask(this->shared_from_this());
    }

    void ProtoProxyTask::OnResponse(const com::Rpc_Response *response)
    {
        auto responseMessage = new c2s::Rpc_Response();
        responseMessage->set_rpcid(this->mClientRpcId);
        responseMessage->set_code(response->code());
        if (response->has_data())
        {
            responseMessage->mutable_data()->CopyFrom(response->data());
        }
        if (this->mProxyComponent->OnResponse(
                this->mClientSockId, responseMessage) != XCode::Successful)
        {
            delete responseMessage;
        }
    }

}