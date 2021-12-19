//
// Created by mac on 2021/11/28.
//

#include"GateComponent.h"
#include"Core/App.h"
#include"Service/RpcNode.h"
#include"Rpc/RpcProxyClient.h"
#include"Scene/RpcConfigComponent.h"
#include"Service/NodeProxyComponent.h"
#include"Async/RpcTask/RpcProxyTask.h"
#include"ServerRpc/RpcComponent.h"
#include"GateClientComponent.h"
#ifdef __DEBUG__
#include<google/protobuf/util/json_util.h>
#endif
namespace GameKeeper
{
    bool GateComponent::Awake()
    {
        this->mRpcConfigComponent = nullptr;
        this->mNodeProxyComponent = nullptr;
        this->mGateClientComponent = nullptr;
        return true;
    }

    bool GateComponent::LateAwake()
    {
        LOG_CHECK_RET_FALSE(this->mRpcComponent = this->GetComponent<RpcComponent>());
        LOG_CHECK_RET_FALSE(this->mRpcConfigComponent = this->GetComponent<RpcConfigComponent>());
        LOG_CHECK_RET_FALSE(this->mNodeProxyComponent = this->GetComponent<NodeProxyComponent>());
        LOG_CHECK_RET_FALSE(this->mGateClientComponent = this->GetComponent<GateClientComponent>());
        return true;
    }

    XCode GateComponent::OnRequest(const c2s::Rpc_Request *request)
    {
        auto config = this->mRpcConfigComponent->GetProtocolConfig(request->methodname());
        if (config == nullptr) {
            LOG_ERROR("call function " << request->methodname() << " not find");
            return XCode::NotFoundRpcConfig;
        }
        auto nodeService = this->mNodeProxyComponent->AllotService(config->Service);
        if (nodeService == nullptr) {
            return XCode::CallServiceNotFound;
        }
        if (!config->Request.empty()) {
            if (!request->has_data()) //没有正确的消息体
            {
                return XCode::CallArgsError;
            }
            this->mProtoName.clear();
            if (Any::ParseAnyTypeUrl(request->data().type_url(), &mProtoName)) {
                if (this->mProtoName != config->Request) //请求的消息不正确
                {
                    return XCode::CallArgsError;
                }
            }
        }

        int methodId = 0;
        auto requestMessage = nodeService->NewRequest(request->methodname(), methodId);
        if (request->has_data()) {
            requestMessage->mutable_data()->CopyFrom(request->data());
        }


        std::shared_ptr<RpcProxyTask> proxyTask(new RpcProxyTask(methodId));
        proxyTask->InitProxyTask(request->rpcid(), request->sockid(), this, this->mRpcComponent);
        return XCode::Successful;
    }

    XCode GateComponent::OnResponse(long long sockId, const c2s::Rpc_Response *response)
    {
        RpcProxyClient * proxyClient = this->mGateClientComponent->GetProxyClient(sockId);
#ifdef __DEBUG__
        std::string json;
        util::MessageToJsonString(*response, &json);
        LOG_WARN("**********[client response]**********");
        LOG_WARN("json = " << json);
        LOG_WARN("*****************************************");
#endif
        if(proxyClient == nullptr || !proxyClient->SendToClient(response))
        {
            return XCode::NetWorkError;
        }
        return XCode::Successful;
    }
}