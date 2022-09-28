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
        TaskComponent * taskComponent = this->GetApp()->GetTaskComponent();


        std::shared_ptr<Rpc::Data> data(Rpc::Data());

        if(!rpcInterfaceConfig->Request.empty())
        {

        }

        std::string userId;
        if(request->GetHead("user_id", userId))
        {
            long long id = std::stoll(userId);
        }

        if(!rpcInterfaceConfig->IsAsync)
        {

            return;
        }
        taskComponent->Start([targetService, method, rpcInterfaceConfig, this, response, httpClient]()
        {

        });
    }
}