//
// Created by zmhy0073 on 2021/12/1.
//

#include"RpcProxyTask.h"
#include"Rpc/RpcComponent.h"
#include"Component/GateComponent.h"
namespace GameKeeper
{
    RpcProxyTask::RpcProxyTask(int methodId)
        : RpcTaskBase(methodId)
    {
        this->mRpcId = 0;
        this->mSockId = 0;
        this->mProxyComponent = nullptr;
    }

    void RpcProxyTask::InitProxyTask(long long rpcId, long long sockId, GateComponent *component,
                                     RpcComponent *rpcComponent)
    {
        this->mRpcId = rpcId;
        this->mSockId = sockId;
        this->mProxyComponent = component;
        rpcComponent->AddRpcTask(this->shared_from_this());
    }

    void RpcProxyTask::OnResponse(const com::Rpc_Response *response)
    {
        auto responseMessage = new c2s::Rpc_Response();
        if(response == nullptr)
        {
            responseMessage->set_code((int)XCode::CallTimeout);
        }
        else
        {
            responseMessage->set_code(response->code());
            responseMessage->set_rpcid(this->mRpcId);
            if (response->has_data())
            {
                responseMessage->mutable_data()->CopyFrom(response->data());
            }
        }
        XCode code = this->mProxyComponent->OnResponse(this->mSockId, responseMessage);
        if(code != XCode::Successful)
        {
            delete responseMessage;
        }
    }

}