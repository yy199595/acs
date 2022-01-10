//
// Created by zmhy0073 on 2021/12/1.
//

#include"RpcProxyTask.h"
#include"Rpc/RpcComponent.h"
#include"Component/GateComponent.h"
namespace GameKeeper
{
    RpcProxyTask::RpcProxyTask()
    {
        this->mRpcId = 0;
        this->mSockId = 0;
        this->mProxyComponent = nullptr;
        this->mTaskRpcId = Helper::Guid::Create();
    }

    void RpcProxyTask::InitProxyTask(long long rpcId, long long sockId, GateComponent *component,
                                     RpcComponent *rpcComponent)
    {
        this->mRpcId = rpcId;
        this->mSockId = sockId;
        this->mProxyComponent = component;
        rpcComponent->AddRpcTask(this->shared_from_this());
    }

    void RpcProxyTask::OnResponse(std::shared_ptr<com::Rpc_Response> response)
    {
        std::shared_ptr<c2s::Rpc_Response> responseMessage(new c2s::Rpc_Response());
        if(response == nullptr)
        {
            responseMessage->set_code((int)XCode::CallTimeout);
        }
        else
        {
            responseMessage->set_code(response->code());
            responseMessage->set_rpc_id(this->mRpcId);
            if (response->has_data())
            {
                responseMessage->mutable_data()->CopyFrom(response->data());
            }
        }
        this->mProxyComponent->OnResponse(this->mSockId, responseMessage);
    }

}