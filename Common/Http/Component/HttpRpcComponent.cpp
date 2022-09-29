//
// Created by zmhy0073 on 2022/8/11.
//

#include"HttpRpcComponent.h"
#include"Client/Message.h"
#include"Service/Service.h"
#include"String/StringHelper.h"
#include"Client/HttpHandlerClient.h"
#include"Component/ProtoComponent.h"
namespace Sentry
{
    void HttpRpcComponent::OnRequest(std::shared_ptr<HttpHandlerClient> httpClient)
    {
        assert(this->GetApp()->IsMainThread());
        std::shared_ptr<HttpHandlerRequest> request = httpClient->Request();
        std::shared_ptr<HttpHandlerResponse> response = httpClient->Response();
        const HttpData & httpData = request->GetData();


        std::vector<std::string> tempArray;
        const ListenConfig & listenConfig = this->GetListenConfig();
        if(Helper::String::Split(httpData.mPath, "/", tempArray) != 2)
        {
            httpClient->StartWriter(HttpStatus::NOT_FOUND);
            return;
        }
        const std::string & method = tempArray[1];
        const std::string & service = tempArray[0];
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
        if(!rpcInterfaceConfig->Request.empty() && httpData.mMethod != "POST")
        {
            httpClient->StartWriter(HttpStatus::METHOD_NOT_ALLOWED);
            return;
        }


        std::shared_ptr<Rpc::Data> data(new Rpc::Data());


        data->SetProto(Tcp::Porto::Json);
        data->SetType(Tcp::Type::Request);
        if(!rpcInterfaceConfig->Request.empty())
        {
            data->Append(request->GetContent());
        }

        std::string userId;
        if(request->GetHead("user_id", userId))
        {
            data->GetHead().Add("id", userId);
        }

        if(!rpcInterfaceConfig->IsAsync)
        {
            XCode code = targetService->Invoke(method, data);

            data->GetHead().Add("code", code);
            return;
        }
        TaskComponent * taskComponent = this->GetApp()->GetTaskComponent();
        taskComponent->Start([targetService, method, rpcInterfaceConfig, this, response, httpClient]()
        {

        });
    }
}