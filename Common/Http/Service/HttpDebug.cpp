//
// Created by zmhy0073 on 2022/8/11.
//

#include"HttpDebug.h"
#include"Client/Message.h"
#include"Service/RpcService.h"
#include"Json/JsonWriter.h"
#include"Config/CodeConfig.h"
#include"String/StringHelper.h"
#include"Config/ClusterConfig.h"
#include"Client/HttpHandlerClient.h"
#include"Component/ProtoComponent.h"
#include"Component/NodeMgrComponent.h"
#include"Component/InnerNetMessageComponent.h"
namespace Sentry
{
    bool HttpDebug::OnStartService(HttpServiceRegister& serviceRegister)
    {
        serviceRegister.Bind("Call", &HttpDebug::Call);
        return true;
    }

    int HttpDebug::Call(const Http::Request& request, Http::Response& response)
    {
        std::string value, func;
        std::vector<std::string> splits;
        std::shared_ptr<Json::Writer> document = std::make_shared<Json::Writer>();
        if (Helper::Str::Split(request.Path(), "/", splits) < 2)
        {
            document->Add("code").Add(XCode::CallArgsError);
            document->Add("error").Add(CodeConfig::Inst()->GetDesc(XCode::CallArgsError));
            response.Json(HttpStatus::OK, *document);
            return XCode::CallArgsError;
        }
        const std::string& method = splits[splits.size() - 1];
        const std::string& service = splits[splits.size() - 2];
        const RpcServiceConfig* rpcServiceConfig = RpcConfig::Inst()->GetConfig(service);
        if (rpcServiceConfig == nullptr)
        {
            document->Add("code").Add((int)XCode::CallServiceNotFound);
            document->Add("error").Add(CodeConfig::Inst()->GetDesc(XCode::CallServiceNotFound));
            response.Json(HttpStatus::OK, *document);
            return XCode::CallServiceNotFound;
        }
        const RpcMethodConfig* methodConfig = rpcServiceConfig->GetMethodConfig(method);
        if (methodConfig == nullptr)
        {
            document->Add("code").Add((int)XCode::CallFunctionNotExist);
            document->Add("error").Add(CodeConfig::Inst()->GetDesc(XCode::CallFunctionNotExist));
            response.Json(HttpStatus::OK, *document);
            return XCode::CallServiceNotFound;
        }

        std::shared_ptr<Rpc::Packet> message(new Rpc::Packet());
        {
            message->GetHead().Add("func", methodConfig->FullName);

            if (request.Header().Get("id", value))
            {
                Helper::Str::ClearBlank(value);
                message->GetHead().Add("id", value);
            }
            message->SetProto(Tcp::Porto::Json);
            message->SetType(Tcp::Type::Request);
            const Http::PostRequest* postRequest = dynamic_cast<const Http::PostRequest*>(&request);
            if (postRequest != nullptr)
            {
                message->Append(postRequest->Content());
            }
        }
        int code;
        try
        {
            code = this->Invoke(message, document);
        }
        catch (std::exception& e)
        {
            code = XCode::ThrowError;
            document->Add("error").Add(e.what());
        }
        std::string json;
        document->Add("code").Add((int)code);
        document->Add("error").Add(CodeConfig::Inst()->GetDesc(code));
        response.Json(HttpStatus::OK, *document);
        return XCode::Successful;
    }

    int HttpDebug::Invoke(std::shared_ptr<Rpc::Packet> data, std::shared_ptr<Json::Writer> document)
    {
        std::string fullName;
        if (!data->GetHead().Get("func", fullName))
        {
            throw std::logic_error("parse func error");
        }
        const RpcMethodConfig* methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
        if (methodConfig == nullptr)
        {
            throw std::logic_error("calling method does not exist");
        }
        long long userId = 0;
        std::string server, address;
        NodeMgrComponent* nodeComponent = this->GetComponent<NodeMgrComponent>();
        ClusterConfig::Inst()->GetServerName(methodConfig->Service, server);
        if (data->GetHead().Get("id", userId))
        {
            nodeComponent->GetServer(server, userId, address);
        }
        else
        {
            nodeComponent->GetServer(server, address);
        }
        if (address.empty())
        {
            throw std::logic_error("get service node error");
        }
        InnerNetMessageComponent* component = this->GetComponent<InnerNetMessageComponent>();
        std::shared_ptr<Rpc::Packet> response = component->Call(address, data);
        if (response == nullptr)
        {
            document->Add("error").Add("unknow error");
        }
        else
        {
            std::string error;
            int code = response->GetCode();
            if (code == XCode::Successful)
            {
                if (!methodConfig->Response.empty())
                {
                    std::unique_ptr<rapidjson::Document> json
                        = std::make_unique<rapidjson::Document>();
                    const std::string& str = response->GetBody();
                    if (json->Parse(str.c_str(), str.size()).HasParseError())
                    {
                        throw std::logic_error("failed to parse the returned data");
                    }
                    document->Add("message").Add(*json);
                }                
            }
            else if (response->GetHead().Get("error", error))
            {
                document->Add("error").Add(error);
            }
            return code;
        }
    }
}