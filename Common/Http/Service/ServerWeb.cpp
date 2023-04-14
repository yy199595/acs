//
// Created by zmhy0073 on 2022/8/29.
//
#include"ServerWeb.h"
#include"Core/System/System.h"
#include"Entity/Unit/App.h"
#include"Server/Config/CodeConfig.h"
#include"Common/Service/Node.h"
#include"Registry/Service/Registry.h"
#include"Util/File/DirectoryHelper.h"
#include"Util/File/FileHelper.h"
#include"Rpc/Component/NodeMgrComponent.h"
#include "Timer/Timer/ElapsedTimer.h"
#include"Cluster/Config/ClusterConfig.h"

namespace Tendo
{
    bool ServerWeb::OnInit()
	{
		BIND_COMMON_HTTP_METHOD(ServerWeb::Info);
		BIND_COMMON_HTTP_METHOD(ServerWeb::Stop);
		BIND_COMMON_HTTP_METHOD(ServerWeb::Login);
		BIND_COMMON_HTTP_METHOD(ServerWeb::Hotfix);
		BIND_COMMON_HTTP_METHOD(ServerWeb::Register);
		return true;
	}

    int ServerWeb::Login(const Http::Request &request, Http::DataResponse &response)
    {
        std::string account,password;
        Http::Parameter parameter(request.Content());
        LOG_ERROR_RETURN_CODE(parameter.Get("account,", account), XCode::CallArgsError);
        LOG_ERROR_RETURN_CODE(parameter.Get("password", password), XCode::CallArgsError);
        return XCode::Successful;
    }

    int ServerWeb::Register(const Http::Request& request, Http::DataResponse& response)
    {
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

	int ServerWeb::Stop(Json::Writer & response)
	{
		std::string address;
		RpcService	* registryService = this->mApp->GetService<Registry>();
		NodeMgrComponent * locationComponent = this->GetComponent<NodeMgrComponent>();
		if(!locationComponent->GetServer(registryService->GetServer(), address))
		{
			return XCode::AddressAllotFailure;
		}
		std::shared_ptr<s2s::server::list> list =
			std::make_shared<s2s::server::list>();
		int code = registryService->Call(address, "Query", list);
		if(code != XCode::Successful)
		{
			response.Add("error").Add(CodeConfig::Inst()->GetDesc(code));
			return XCode::Failure;
		}
		std::string local;
		ServerConfig::Inst()->GetLocation("rpc", local);
		RpcService * nodeService = this->GetComponent<Node>();
		std::vector<const s2s::server::info *> serverInfos;
		for (int index = 0; index < list->list_size(); index++)
		{
			const s2s::server::info& info = list->list(index);
			serverInfos.emplace_back(&info);
		}
		std::sort(serverInfos.begin(), serverInfos.end(), []
			(const s2s::server::info * info1, const s2s::server::info * info2)->bool
		{
				const NodeConfig * config1 = ClusterConfig::Inst()->GetConfig(info1->name());
				const NodeConfig * config2 = ClusterConfig::Inst()->GetConfig(info2->name());
				if(config1 != nullptr && config2 != nullptr)
				{
					return config1->GetIndex() < config2->GetIndex();
				}
				return true;
		});
		for(const s2s::server::info * info : serverInfos)
		{
			if(info->rpc() != local)
			{
				ElapsedTimer timer;
				int code = nodeService->Call(info->rpc(), "Stop");
				response.BeginObject(info->rpc().c_str());
				response.Add("rpc").Add(info->rpc());
				response.Add("name").Add(info->name());
				response.Add("http").Add(info->http());
				response.Add("ms").Add(timer.GetMs());
				response.Add("code").Add(CodeConfig::Inst()->GetDesc(code));
				response.EndObject();
			}
		}
		this->mApp->GetTaskComponent()->Start(&App::Stop, this->mApp, 0);
		return XCode::Successful;
	}

	int ServerWeb::Info(Json::Writer&response)
    {
		std::string address;
		RpcService	* registryService = this->mApp->GetService<Registry>();
		NodeMgrComponent * locationComponent = this->GetComponent<NodeMgrComponent>();
		if(!locationComponent->GetServer(registryService->GetServer(), address))
		{
			return XCode::AddressAllotFailure;
		}
		s2s::server::query message;
		std::shared_ptr<s2s::server::list> list =
			std::make_shared<s2s::server::list>();
		int code = registryService->Call(address, "Query", message, list);
		if(code != XCode::Successful)
		{
			response.Add("error").Add(CodeConfig::Inst()->GetDesc(code));
			return XCode::Failure;
		}
		const std::string method("RunInfo");
		RpcService * nodeService = this->GetComponent<Node>();
		for (int index = 0; index < list->list_size(); index++)
		{
			const s2s::server::info& info = list->list(index);
			std::shared_ptr<com::type::string> resp =
				std::make_shared<com::type::string>();
			int code = nodeService->Call(info.rpc(), method, resp);
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