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

    XCode HttpRpcService::Call(const Http::Request &request, Http::Response &response)
    {
        std::string value, func;
        std::vector<std::string> splits;
        std::shared_ptr<Json::Writer> document = std::make_shared<Json::Writer>();
        if (Helper::String::Split(request.Path(), "/", splits) < 2)
        {            
            document->Add("code").Add((int)XCode::CallArgsError);
            document->Add("error").Add(CodeConfig::Inst()->GetDesc(XCode::CallArgsError));
            response.Json(HttpStatus::OK, *document);
            return XCode::CallArgsError;
        }
        const std::string &method = splits[splits.size() - 1];
        const std::string &service = splits[splits.size() - 2];
        const RpcServiceConfig *rpcServiceConfig = RpcConfig::Inst()->GetConfig(service);
        if (rpcServiceConfig == nullptr)
        {          
            document->Add("code").Add((int) XCode::CallServiceNotFound);
            document->Add("error").Add(CodeConfig::Inst()->GetDesc(XCode::CallServiceNotFound));
            response.Json(HttpStatus::OK, *document);
            return XCode::CallServiceNotFound;
        }
        const RpcMethodConfig *methodConfig = rpcServiceConfig->GetMethodConfig(method);
        if (methodConfig == nullptr)
        {           
            document->Add("code").Add((int) XCode::CallFunctionNotExist);
            document->Add("error").Add(CodeConfig::Inst()->GetDesc(XCode::CallFunctionNotExist));
            response.Json(HttpStatus::OK, *document);
            return XCode::CallServiceNotFound;
        }

        std::shared_ptr<Rpc::Packet> message(new Rpc::Packet());
        {
            message->GetHead().Add("func", methodConfig->FullName);

            if (request.Header().Get("id", value))
            {
                Helper::String::ClearBlank(value);
                message->GetHead().Add("id", value);
            }
            message->SetProto(Tcp::Porto::Json);
            message->SetType(Tcp::Type::Request);
            const Http::PostRequest * postRequest = dynamic_cast<const Http::PostRequest*>(&request);
            if(postRequest != nullptr)
            {
                message->Append(postRequest->Content());
            }
        }
        XCode code = XCode::Failure;
        try
        {
            code = this->Invoke(message, document);
        }
        catch (std::exception &e)
        {
            code = XCode::ThrowError;
            document->Add("error").Add(e.what());
        }
        std::string json;
        document->Add("code").Add((int) code);
		document->Add("error").Add(CodeConfig::Inst()->GetDesc(code));
		response.Json(HttpStatus::OK, *document);
        return XCode::Successful;
    }

    XCode HttpRpcService::Invoke(std::shared_ptr<Rpc::Packet> data, std::shared_ptr<Json::Writer> document)
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
                std::unique_ptr<rapidjson::Document> json
                    = std::make_unique<rapidjson::Document>();
                const std::string &str = data->GetBody();
                if (json->Parse(str.c_str(), str.size()).HasParseError())
                {
                    throw std::logic_error("failed to parse the returned data");
                }
                document->Add("message").Add(*json);
            }
            else if(data->GetBody().empty())
            {
                std::unique_ptr<rapidjson::Document> json
                    = std::make_unique<rapidjson::Document>();
                const char * str = data->GetBody().c_str();
                const size_t length = data->GetBody().size();
                if(!json->Parse(str, length).HasParseError())
                {
                    document->Add("data").Add(*json);
                }
            }
        }
        return code;
    }
}