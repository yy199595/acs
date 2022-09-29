//
// Created by zmhy0073 on 2022/8/11.
//

#include"HttpRpcComponent.h"
#include"Client/Message.h"
#include"Service/Service.h"
#include"Json/JsonWriter.h"
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
        const RpcMethodConfig* methodConfig =  config.GetConfig(method);
        if(methodConfig == nullptr)
        {
            httpClient->StartWriter(HttpStatus::NOT_FOUND);
            return;
        }
        if(!methodConfig->Request.empty() && httpData.mMethod != "POST")
        {
            httpClient->StartWriter(HttpStatus::METHOD_NOT_ALLOWED);
            return;
        }


        std::shared_ptr<Rpc::Data> data(new Rpc::Data());


        data->SetProto(Tcp::Porto::Json);
        data->SetType(Tcp::Type::Request);
        data->Append(request->GetContent());

        std::string userId;
        if(request->GetHead("user_id", userId))
        {
            data->GetHead().Add("id", userId);
        }

        if(!methodConfig->IsAsync)
        {
            this->Invoke(httpClient, methodConfig, data);
            return;
        }
        TaskComponent * taskComponent = this->GetApp()->GetTaskComponent();
        taskComponent->Start(&HttpRpcComponent::Invoke, this, httpClient, methodConfig, data);
    }

    void HttpRpcComponent::Invoke(std::shared_ptr<HttpHandlerClient> httpClient,
                                  const RpcMethodConfig * config, std::shared_ptr<Rpc::Data> message)
    {
        XCode code = XCode::ThrowError;
        std::shared_ptr<HttpHandlerResponse> response = httpClient->Response();
        Service * targetService = this->GetApp()->GetService(config->Service);
        try
        {
            const std::string & method = config->Method;
            code = targetService->Invoke(method, message);
            if(code == XCode::Successful && !config->Response.empty())
            {
                response->WriteString(message->GetBody());
            }
        }
        catch (std::exception & e)
        {
            response->AddHead("error", e.what());
        }
        response->AddHead("code", (int)code);
        httpClient->StartWriter(HttpStatus::OK);
    }
}