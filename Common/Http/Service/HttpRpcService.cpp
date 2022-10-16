//
// Created by zmhy0073 on 2022/8/11.
//

#include"HttpRpcService.h"
#include"Client/Message.h"
#include"Service/RpcService.h"
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
        const size_t pos = httpData.mPath.find_last_of('/');

        std::shared_ptr<Json::Document> document
            = std::make_shared<Json::Document>();
        if(pos == std::string::npos)
        {
            document->Add("error", "调用方法错误");
            document->Add("code", (int) XCode::CallServiceNotFound);

            document->Serialize(response.Content());
            return XCode::CallServiceNotFound;
        }
        func = httpData.mPath.substr(pos + 1);
        message->GetHead().Add("func", func);

        if(httpData.Get("id", value))
        {
            Helper::String::ClearBlank(value);
            message->GetHead().Add("id", value);
        }

        message->SetProto(Tcp::Porto::Json);
        message->SetType(Tcp::Type::Request);
        message->Append(request.GetContent());

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
        document->Serialize(response.Content());
        return XCode::Successful;
    }

    XCode HttpRpcService::Invoke(std::shared_ptr<Rpc::Data> data, std::shared_ptr<Json::Document> document)
    {
        std::string fullName;
        if(!data->GetHead().Get("func", fullName))
        {
            throw std::logic_error("parse func error");
        }
        const RpcMethodConfig* methodConfig =  ServiceConfig::Inst()->GetRpcMethodConfig(fullName);
        if(methodConfig == nullptr)
        {
            throw std::logic_error("calling method does not exist");
        }

        if(!methodConfig->Request.empty() && data->GetBody().empty())
        {
            throw std::logic_error("request parameter cannot be empty");
        }

        RpcService * targetService = this->mApp->GetService(methodConfig->Service);
        if(targetService == nullptr || !targetService->IsStartService())
        {
            throw std::logic_error("calling service does not exist or is not started");           
        }

        XCode code = targetService->Invoke(methodConfig->Method, data);
        if(code == XCode::Successful && !methodConfig->Response.empty())
        {
            rapidjson::Document json;
            const std::string & str = data->GetBody();
            if(json.Parse(str.c_str(), str.size()).HasParseError())
            {
                throw std::logic_error("failed to parse the returned data");
            }
            document->Add("message", json);
        }
        return code;
    }
}