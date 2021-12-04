//
// Created by mac on 2021/11/28.
//

#include"ProtoProxyComponent.h"
#include"Core/App.h"
#include"Service/RpcNodeProxy.h"
#include"Scene/RpcConfigComponent.h"
#include"Service/NodeProxyComponent.h"
#include"Async/RpcTask/ProtoProxyTask.h"
#include"ProtoRpc/ProtoRpcComponent.h"
#include"Rpc/RpcProxyClient.h"
#include"ProxyRpc/ProtoProxyClientComponent.h"
#ifdef __DEBUG__
#include<google/protobuf/util/json_util.h>
#endif
namespace GameKeeper
{
    ProtoProxyComponent::ProtoProxyComponent()
    {
        this->mRpcConfigComponent = nullptr;
        this->mNodeProxyComponent = nullptr;
        this->mProxyClientComponent = nullptr;
    }

    bool ProtoProxyComponent::Awake()
    {
        LOG_CHECK_RET_FALSE(this->mRpcComponent = this->GetComponent<ProtoRpcComponent>());
        LOG_CHECK_RET_FALSE(this->mRpcConfigComponent = this->GetComponent<RpcConfigComponent>());
        LOG_CHECK_RET_FALSE(this->mNodeProxyComponent = this->GetComponent<NodeProxyComponent>());
        LOG_CHECK_RET_FALSE(this->mProxyClientComponent = this->GetComponent<ProtoProxyClientComponent>());
        return true;
    }

    XCode ProtoProxyComponent::OnRequest(const c2s::Rpc_Request *request)
    {
        auto config = this->mRpcConfigComponent->GetProtocolConfig(request->methodname());
        if (config == nullptr)
        {
            LOG_ERROR("call function " << request->methodname() << " not find");
            return XCode::NotFoundRpcConfig;
        }
        auto nodeService = this->mNodeProxyComponent->AllotService(config->Service);
        if (nodeService == nullptr)
        {
            return XCode::CallServiceNotFound;
        }
        if (!config->Request.empty())
        {
            if (!request->has_data()) //没有正确的消息体
            {
                return XCode::CallArgsError;
            }
            this->mProtoName.clear();
            if (Any::ParseAnyTypeUrl(request->data().type_url(), &mProtoName))
            {
                if (this->mProtoName != config->Request) //请求的消息不正确
                {
                    return XCode::CallArgsError;
                }
            }
        }

        int methodId = 0;
        auto requestMessage = nodeService->CreateProtoRequest(request->methodname(), methodId);
        if (request->has_data())
        {
            requestMessage->mutable_data()->CopyFrom(request->data());
        }

        if(request->rpcid() != 0)
        {
            long long rpcId = requestMessage->rpcid();
            std::shared_ptr<ProtoProxyTask> proxyTask(new ProtoProxyTask(methodId, rpcId));
            proxyTask->InitProxyTask(request->rpcid(), request->sockid(), this, this->mRpcComponent);
        }

        nodeService->SendRequestData(requestMessage);
        return XCode::Successful;
    }

    XCode ProtoProxyComponent::OnResponse(long long sockId, const c2s::Rpc_Response *response)
    {
        RpcProxyClient * proxyClient = this->mProxyClientComponent->GetProxyClient(sockId);
#ifdef __DEBUG__
        std::string json;
        util::MessageToJsonString(*response, &json);
        LOG_WARN("**********[client response]**********");
        LOG_WARN("json = " << json);
        LOG_WARN("*****************************************");
#endif
        if(proxyClient == nullptr || !proxyClient->StartSendData(RPC_TYPE_RESPONSE, response))
        {
            return XCode::NetWorkError;
        }
        return XCode::Successful;
    }
}