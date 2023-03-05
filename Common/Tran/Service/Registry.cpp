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
		BIND_COMMON_RPC_METHOD(Registry::Register);
        BIND_COMMON_RPC_METHOD(Registry::UnRegister);
		this->mLocationComponent = this->GetComponent<NodeMgrComponent>();
        return true;
    }

    bool Registry::OnClose()
    {
        return true;
    }

	int Registry::Register(const s2s::server::info& request, s2s::server::list& response)
	{
		const std::string & rpc = request.rpc();
		const std::string & http = request.http();
		const std::string & name = request.name();
		RpcService * rpcService = this->mApp->GetService<Node>();
		this->mLocationComponent->AddRpcServer(name, rpc);
		this->mLocationComponent->AddHttpServer(name, http);
		InnerNetComponent * innerNetComponent = this->GetComponent<InnerNetComponent>();
		if(innerNetComponent != nullptr)
		{
            const std::string func("Join");
            std::vector<const ServiceNodeInfo *> services;
			innerNetComponent->GetServiceList(services);
			for(const ServiceNodeInfo * nodeInfo : services)
			{
				const std::string & address = nodeInfo->LocationRpc;
				if(rpcService->Call(address, func, request) == XCode::Successful)
				{
                    s2s::server::info * server = response.add_list();
                    {
                        server->set_name(nodeInfo->SrvName);
                        server->set_rpc(nodeInfo->LocationRpc);
                        server->set_http(nodeInfo->LocationHttp);
                    }
				}
			}
			const ServerConfig * config = ServerConfig::Inst();
			s2s::server::info * localServer = response.add_list();
			localServer->set_name(config->Name());
			config->GetLocation("rpc", *localServer->mutable_rpc());
			config->GetLocation("http", *localServer->mutable_http());
		}
		return XCode::Successful;
	}

	int Registry::UnRegister(const com::type::string& request)
	{
		return XCode::Successful;
	}
}