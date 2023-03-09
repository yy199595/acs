//
// Created by zmhy0073 on 2022/8/29.
//
#include"HttpBackGround.h"
#include"System/System.h"
#include"Config/CodeConfig.h"
#include"Service/Node.h"
#include"Service/Registry.h"
#include"google/protobuf/wrappers.pb.h"
#include"Component/NodeMgrComponent.h"
namespace Sentry
{
    bool HttpBackGround::OnStartService(HttpServiceRegister &serviceRegister)
    {
        serviceRegister.Bind("Info", &HttpBackGround::Info);
        serviceRegister.Bind("Ping", &HttpBackGround::Ping);
        serviceRegister.Bind("Hello", &HttpBackGround::Hello);
		serviceRegister.Bind("Sleep", &HttpBackGround::Sleep);
		serviceRegister.Bind("Hotfix", &HttpBackGround::Hotfix);
		serviceRegister.Bind("DownLoad", &HttpBackGround::DownLoad);
        return true;
    }

	int HttpBackGround::Ping(const Http::Request &request, Http::Response &response)
    {
        response.Str(HttpStatus::OK,"pong");
        return XCode::Successful;
    }

	int HttpBackGround::Hotfix(Json::Writer&response)
	{
		RpcService * rpcService = this->mApp->GetService<Node>();
		NodeMgrComponent * locationComponent = this->GetComponent<NodeMgrComponent>();
		if(locationComponent == nullptr || rpcService == nullptr)
		{
			response.Add("error").Add("LocationComponent or InnerService Is Null");
			return XCode::Failure;
		}
		std::vector<std::string> locations;
		locationComponent->GetServers(locations);
		for(const std::string & location : locations)
		{
			int code = rpcService->Call(location, "Hotfix");
            response.Add(location.c_str()).Add(CodeConfig::Inst()->GetDesc(code));
		}
		return XCode::Successful;
	}

	int HttpBackGround::Sleep(const Json::Reader &request, Json::Writer&response)
    {
        std::string time;
        request.GetMember("time", time);
        this->mApp->GetTaskComponent()->Sleep(std::stoi(time));
        response.Add("time").Add(std::stoi(time));
        return XCode::Successful;
    }

	int HttpBackGround::Hello(const Http::Request &request, Http::Response &response)
    {
		response.Str(HttpStatus::OK,"hello");
		return XCode::Successful;
    }

	int HttpBackGround::DownLoad(const Http::Request &request, Http::Response &response)
    {
        return XCode::Successful;
    }

	int HttpBackGround::Info(Json::Writer&response)
    {
		std::string address;
		Node * innerService = this->GetComponent<Node>();
		NodeMgrComponent * locationComponent = this->GetComponent<NodeMgrComponent>();
		if (!locationComponent->GetRegistryAddress(address))
		{
			response.Add("error").Add("not find registry server address");
			return XCode::NetWorkError;
		}
		com::array::string request;
		std::shared_ptr<s2s::server::list> list
			= std::make_shared<s2s::server::list>();
		RpcService* rpcService = this->mApp->GetService<Registry>();		
		int code = rpcService->Call(address, std::string("Query"), request, list);
		if(code != XCode::Successful)
		{
			response.Add("error").Add(CodeConfig::Inst()->GetDesc(code));
			return XCode::Failure;
		}
		for (int index = 0; index < list->list_size(); index++)
		{
			const s2s::server::info& info = list->list(index);
			std::shared_ptr<google::protobuf::StringValue> resp
				= std::make_shared<google::protobuf::StringValue>();
			int code = innerService->Call(info.rpc(), "RunInfo", resp);
			const std::string& desc = CodeConfig::Inst()->GetDesc(code);
			if (code == XCode::Successful)
			{
				rapidjson::Document document;
				const std::string& json = resp->value();
				if (!document.Parse(json.c_str(), json.size()).HasParseError())
				{					
					response.Add(info.rpc()).Add(document);
				}
			}
			else
			{
				response.Add(info.rpc()).Add(desc);
			}
		}
		return XCode::Successful;
    }

}