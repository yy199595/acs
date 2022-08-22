//
// Created by zmhy0073 on 2022/8/11.
//

#include"HttpRpcComponent.h"
#include"Util/StringHelper.h"
#include"Component/RpcService/Service.h"
#include"Network/Http/HttpHandlerClient.h"
#include"Component/Scene/MessageComponent.h"
namespace Sentry
{
    void HttpRpcComponent::OnRequest(std::shared_ptr<HttpHandlerClient> httpClient)
    {
        assert(this->GetApp()->IsMainThread());
        std::shared_ptr<HttpHandlerRequest> request = httpClient->Request();
        std::shared_ptr<HttpHandlerResponse> response = httpClient->Response();


        const std::string & path = request->GetPath();
        const std::string & address = request->GetAddress();

        if (request->GetMethod() != "POST")
        {
            httpClient->StartWriter(HttpStatus::METHOD_NOT_ALLOWED);
            return;
        }
        if(request->GetPath() != "/logic/rpc")
        {
            httpClient->StartWriter(HttpStatus::NOT_FOUND);
            return;
        }
        std::string fullName, service, method;
        if(!request->GetHead("method", fullName))
        {
            httpClient->StartWriter(HttpStatus::BAD_REQUEST);
            return;
        }

        Helper::String::ClearBlank(fullName);
        if(!RpcServiceConfig::ParseFunName(fullName, service, method))
        {
            httpClient->StartWriter(HttpStatus::NOT_FOUND);
            return;
        }
        Service * targetService = this->GetApp()->GetService(service);
        if(targetService == nullptr || !targetService->IsStartService())
        {
            httpClient->StartWriter(HttpStatus::NOT_FOUND);
            return;
        }
        const RpcServiceConfig & config = targetService->GetServiceConfig();
        const RpcInterfaceConfig* rpcInterfaceConfig =  config.GetConfig(method);
        if(rpcInterfaceConfig == nullptr)
        {
            httpClient->StartWriter(HttpStatus::NOT_FOUND);
            return;
        }
        TaskComponent * taskComponent = this->GetApp()->GetTaskComponent();
        std::shared_ptr<com::rpc::request> rpcRequest(new com::rpc::request());

        if(!rpcInterfaceConfig->Request.empty())
        {
            rpcRequest->set_json(request->GetContent());
            rpcRequest->set_type(com::rpc_msg_type_json);
        }

        std::string userId;
        if(request->GetHead("user_id", userId))
        {
            long long id = std::stoll(userId);
            rpcRequest->set_user_id(id);
        }

        if(!rpcInterfaceConfig->IsAsync)
        {
            std::shared_ptr<com::rpc::response> rpcResponse(new com::rpc::response());
            XCode code = targetService->Invoke(method, rpcRequest, rpcResponse);
            if(code == XCode::Successful && !rpcInterfaceConfig->Response.empty())
            {
                if(!rpcResponse->json().empty())
                {
                    response->WriteString(rpcResponse->json());
                }
            }
            response->AddHead("code", (int)code);
            httpClient->StartWriter(HttpStatus::OK);
            return;
        }
        taskComponent->Start([targetService, method, rpcRequest, rpcInterfaceConfig, this, response, httpClient]()
        {
            std::shared_ptr<com::rpc::response> rpcResponse(new com::rpc::response());
            XCode code = targetService->Invoke(method, rpcRequest, rpcResponse);
            if(code == XCode::Successful && !rpcInterfaceConfig->Response.empty())
            {
                if(!rpcResponse->json().empty())
                {
                    response->WriteString(rpcResponse->json());
                }
            }
            response->AddHead("code", (int)code);
            httpClient->StartWriter(HttpStatus::OK);
        });
    }

    HttpStatus HttpRpcComponent::Invoke(std::shared_ptr<HttpHandlerClient> httpClient)
    {

        return HttpStatus::OK;
    }
}