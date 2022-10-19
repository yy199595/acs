//
// Created by zmhy0073 on 2022/8/11.
//

#include"HttpRpcService.h"
#include"Client/Message.h"
#include"Service/RpcService.h"
#include"Json/JsonWriter.h"
#include"Config/CodeConfig.h"
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
        std::vector<std::string> splits;
        const HttpData &httpData = request.GetData();
        std::shared_ptr<Json::Document> document = std::make_shared<Json::Document>();
        if (Helper::String::Split(httpData.mPath, "/", splits) < 2)
        {            
            document->Add("code", (int) XCode::CallArgsError);
            document->Add("error", CodeConfig::Inst()->GetDesc(XCode::CallArgsError));
            document->Serialize(response.Content());
            return XCode::CallArgsError;
        }
        const std::string &method = splits[splits.size() - 1];
        const std::string &service = splits[splits.size() - 2];
        const RpcServiceConfig *rpcServiceConfig = RpcConfig::Inst()->GetConfig(service);
        if (rpcServiceConfig == nullptr)
        {          
            document->Add("code", (int) XCode::CallServiceNotFound);
            document->Add("error", CodeConfig::Inst()->GetDesc(XCode::CallServiceNotFound));
            document->Serialize(response.Content());
            return XCode::CallServiceNotFound;
        }
        const RpcMethodConfig *methodConfig = rpcServiceConfig->GetMethodConfig(method);
        if (methodConfig == nullptr)
        {           
            document->Add("code", (int) XCode::CallFunctionNotExist);
            document->Add("error", CodeConfig::Inst()->GetDesc(XCode::CallFunctionNotExist));
            document->Serialize(response.Content());
            return XCode::CallServiceNotFound;
        }

        std::shared_ptr<Rpc::Packet> message(new Rpc::Packet());
        {
            message->GetHead().Add("func", methodConfig->FullName);

            if (httpData.Get("id", value))
            {
                Helper::String::ClearBlank(value);
                message->GetHead().Add("id", value);
            }
            message->SetProto(Tcp::Porto::Json);
            message->SetType(Tcp::Type::Request);
            message->Append(request.GetContent());
        }
        XCode code = XCode::Failure;
        try
        {
            code = this->Invoke(message, document);
        }
        catch (std::exception &e)
        {
            code = XCode::ThrowError;
            document->Add("error", e.what());
        }
        std::string json;
        document->Add("code", (int) code);
        document->Serialize(response.Content());
        return XCode::Successful;
    }

    XCode HttpRpcService::Invoke(std::shared_ptr<Rpc::Packet> data, std::shared_ptr<Json::Document> document)
    {
        std::string fullName;
        if(!data->GetHead().Get("func", fullName))
        {
            throw std::logic_error("parse func error");
        }
        const RpcMethodConfig* methodConfig =  RpcConfig::Inst()->GetMethodConfig(fullName);
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
        if(code == XCode::Successful)
        {
            if(!methodConfig->Response.empty())
            {
                rapidjson::Document json;
                const std::string &str = data->GetBody();
                if (json.Parse(str.c_str(), str.size()).HasParseError())
                {
                    throw std::logic_error("failed to parse the returned data");
                }
                document->Add("message", json);
            }
            else if(data->GetBody().empty())
            {
                rapidjson::Document json;
                const char * str = data->GetBody().c_str();
                const size_t length = data->GetBody().size();
                if(!json.Parse(str, length).HasParseError())
                {
                    document->Add("data", json);
                }
            }
        }
        return code;
    }
}