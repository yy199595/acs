//
// Created by zmhy0073 on 2022/8/29.
//
#include"ServerWeb.h"
#include"System/System.h"
#include"App/App.h"
#include"Config/CodeConfig.h"
#include"Service/Node.h"
#include"Service/Registry.h"
#include"Component/NodeMgrComponent.h"
namespace Sentry
{
    bool ServerWeb::OnStartService(HttpServiceRegister &serviceRegister)
    {
        serviceRegister.Bind("Info", &ServerWeb::Info);
        serviceRegister.Bind("Ping", &ServerWeb::Ping);
        serviceRegister.Bind("Hello", &ServerWeb::Hello);
		serviceRegister.Bind("Sleep", &ServerWeb::Sleep);
		serviceRegister.Bind("Hotfix", &ServerWeb::Hotfix);
		serviceRegister.Bind("DownLoad", &ServerWeb::DownLoad);
        return true;
    }

	int ServerWeb::Ping(const Http::Request &request, Http::Response &response)
    {
        response.Str(HttpStatus::OK,"pong");
        return XCode::Successful;
    }

	int ServerWeb::Hotfix(Json::Writer&response)
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

	int ServerWeb::Sleep(const Json::Reader &request, Json::Writer&response)
    {
        std::string time;
        request.GetMember("time", time);
        this->mApp->GetTaskComponent()->Sleep(std::stoi(time));
        response.Add("time").Add(std::stoi(time));
        return XCode::Successful;
    }

	int ServerWeb::Hello(const Http::Request &request, Http::Response &response)
    {
		response.Str(HttpStatus::OK,"hello");
		return XCode::Successful;
    }

	int ServerWeb::DownLoad(const Http::Request &request, Http::Response &response)
    {
        return XCode::Successful;
    }

	int ServerWeb::Info(Json::Writer&response)
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
			std::shared_ptr<com::type::string> resp
				= std::make_shared<com::type::string>();
			int code = innerService->Call(info.rpc(), "RunInfo", resp);
			const std::string& desc = CodeConfig::Inst()->GetDesc(code);
			if (code == XCode::Successful)
			{
				rapidjson::Document document;
				const std::string& json = resp->str();
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