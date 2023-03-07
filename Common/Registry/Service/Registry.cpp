//
// Created by zmhy0073 on 2022/10/25.
//

#include"Registry.h"
#include"Service/Node.h"
#include"Component/InnerNetComponent.h"
#include"Component/NodeMgrComponent.h"
namespace Sentry
{
    bool Registry::OnStart()
    {
		BIND_COMMON_RPC_METHOD(Registry::Ping);
		BIND_COMMON_RPC_METHOD(Registry::Query);
		BIND_COMMON_RPC_METHOD(Registry::Register);
        BIND_COMMON_RPC_METHOD(Registry::UnRegister);
		this->mNodeComponent = this->GetComponent<NodeMgrComponent>();
		this->mInnerComponent = this->GetComponent<InnerNetComponent>();		
        return true;
    }

    bool Registry::OnClose()
    {
        return true;
    }

	int Registry::Query(const com::type::string& request, s2s::server::list& response)
	{		
		const std::string& name = request.str();
		std::vector<const ServiceNodeInfo*> services;		
		this->mInnerComponent->GetServiceList(name, services);
		for (const ServiceNodeInfo* nodeInfo : services)
		{
			s2s::server::info * info = response.add_list();
			{
				info->set_name(nodeInfo->SrvName);
				info->set_rpc(nodeInfo->RpcAddress);
				info->set_http(nodeInfo->HttpAddress);
			}
		}
		return XCode::Successful;
	}

	int Registry::Register(const s2s::server::info& request, s2s::server::list& response)
	{
		const std::string& rpc = request.rpc();
		const std::string& http = request.http();
		const std::string& name = request.name();
		this->mNodeComponent->AddRpcServer(name, rpc);
		this->mNodeComponent->AddHttpServer(name, http);
		RpcService* rpcService = this->mApp->GetService<Node>();
		
		const std::string func("Join");
		std::vector<const ServiceNodeInfo*> services;
		this->mInnerComponent->GetServiceList(services);
		for (const ServiceNodeInfo* nodeInfo : services)
		{
			const std::string& address = nodeInfo->RpcAddress;
			if (rpcService->Call(address, func, request) == XCode::Successful)
			{
				s2s::server::info* server = response.add_list();
				{
					server->set_name(nodeInfo->SrvName);
					server->set_rpc(nodeInfo->RpcAddress);
					server->set_http(nodeInfo->HttpAddress);
				}
			}
		}
		const ServerConfig* config = ServerConfig::Inst();
		s2s::server::info* localServer = response.add_list();
		{
			localServer->set_name(config->Name());
			config->GetLocation("rpc", *localServer->mutable_rpc());
			config->GetLocation("http", *localServer->mutable_http());
		}
		return XCode::Successful;
	}

	void Registry::OnNodeServerError(const std::string& address)
	{

	}

	int Registry::Ping(const Rpc::Head& head)
	{
		return 0;
	}

	int Registry::UnRegister(const com::type::string& request)
	{
		return XCode::Successful;
	}
}