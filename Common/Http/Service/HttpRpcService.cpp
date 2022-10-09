//
// Created by zmhy0073 on 2022/8/11.
//

#include"HttpRpcService.h"
#include"Client/Message.h"
#include"Service/Service.h"
#include"Json/JsonWriter.h"
#include"String/StringHelper.h"
#include"Client/HttpHandlerClient.h"
#include"Component/ProtoComponent.h"
namespace Sentry
{
    bool HttpRpcService::OnStartService(HttpServiceRegister &serviceRegister)
    {
        serviceRegister.Bind("Call", &HttpRpcService::Call);
        return true;
    }

    XCode HttpRpcService::Call(const HttpHandlerRequest &request, HttpHandlerResponse &response)
    {
        std::string value, func;
        const HttpData & httpData = request.GetData();
        std::shared_ptr<Rpc::Data> message(new Rpc::Data());
        if(httpData.Get("func", func))
        {
            Helper::String::ClearBlank(func);
            message->GetHead().Add("func", func);
        }
        else
        {
            response.WriteString("请求头中没有func字段");
            return XCode::CallArgsError;
        }

        if(httpData.Get("id", value))
        {
            Helper::String::ClearBlank(value);
            message->GetHead().Add("id", value);
        }

        message->SetProto(Tcp::Porto::Json);
        message->SetType(Tcp::Type::Request);
        message->Append(request.GetContent());
        std::shared_ptr<Json::Document> document
            = std::make_shared<Json::Document>();

        XCode code = XCode::Failure;
        try
        {
            code = this->Invoke(message, document);
        }
        catch (std::exception & e)
        {
            code = XCode::ThrowError;
            document->Add("error", e.what());
        }
        std::string json;
        document->Add("code", (int)code);
        response.WriteString(*document->Serialize(&json));
        return XCode::Successful;
    }

    XCode HttpRpcService::Invoke(std::shared_ptr<Rpc::Data> data, std::shared_ptr<Json::Document> document)
    {
        std::string service, method;
        if(!data->GetMethod(service, method))
        {
            throw std::logic_error("parse func error");
        }

        Service * targetService = this->GetApp()->GetService(service);
        if(targetService == nullptr || !targetService->IsStartService())
        {
            throw std::logic_error("调用服务不存在或者没有启动");
            return XCode::CallServiceNotFound;
        }
        const RpcServiceConfig & config = targetService->GetServiceConfig();
        const RpcMethodConfig* methodConfig =  config.GetConfig(method);
        if(methodConfig == nullptr)
        {
            throw std::logic_error("调用方法不存在");
            return XCode::CallServiceNotFound;
        }
        if(!methodConfig->Request.empty() && data->GetBody().empty())
        {
            throw std::logic_error("请求参数不能为空");
            return XCode::CallArgsError;
        }
        return targetService->Invoke(method, data);
    }
}