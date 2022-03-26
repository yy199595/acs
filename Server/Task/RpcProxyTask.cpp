//
// Created by zmhy0073 on 2021/12/1.
//

#include"RpcProxyTask.h"
#include"Component/Rpc/RpcComponent.h"
#include"Component/Gate/GateComponent.h"
namespace Sentry
{
    RpcProxyTask::RpcProxyTask()
    {
        this->mRpcId = 0;
        this->mSockId = 0;
        this->mGateComponent = nullptr;
        this->mTaskRpcId = Helper::Guid::Create();
    }

    void RpcProxyTask::InitProxyTask(long long rpcId, long long sockId, GateComponent *component,
                                     RpcComponent *rpcComponent)
    {
        this->mRpcId = rpcId;
        this->mSockId = sockId;
        this->mGateComponent = component;
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
            responseMessage->set_rpc_id(this->mRpcId);
            responseMessage->set_code(response->code());
            responseMessage->set_error_str(response->error_str());
            if (response->has_data())
            {
                responseMessage->mutable_data()->CopyFrom(response->data());
            }
        }
        this->mGateComponent->OnResponse(this->mSockId, responseMessage);
    }

}