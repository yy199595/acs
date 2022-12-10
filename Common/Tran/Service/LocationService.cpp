//
// Created by zmhy0073 on 2022/10/25.
//

#include"LocationService.h"
#include"Service/InnerService.h"
#include"Component/InnerNetComponent.h"
#include"Component/LocationComponent.h"
namespace Sentry
{
    bool LocationService::OnStart()
    {
        BIND_COMMON_RPC_METHOD(LocationService::Add);
        BIND_COMMON_RPC_METHOD(LocationService::Del);
		BIND_COMMON_RPC_METHOD(LocationService::Register);
		this->mLocationComponent = this->GetComponent<LocationComponent>();
        return true;
    }

    bool LocationService::OnClose()
    {
        return true;
    }

    XCode LocationService::Add(const s2s::location::add &request)
    {
        long long userId = request.user_id();
        std::unique_ptr<LocationUnit> locationUnit(new LocationUnit(userId));
        for(auto & value : request.services())
        {
            locationUnit->Add(value.first, value.second);
        }
        this->mLocationComponent->AddUnit(std::move(locationUnit));
        return XCode::Successful;
    }

    XCode LocationService::Del(const s2s::location::del &request)
    {
        long long userId = request.user_id();
        LocationUnit *locationUnit = this->mLocationComponent->GetUnit(userId);
        if (locationUnit == nullptr)
        {
            return XCode::NotFindUser;
        }

        if (request.services_size() == 0)
        {
            this->mLocationComponent->DelUnit(userId);
            return XCode::Successful;
        }

        for (const std::string &service: request.services())
        {
            locationUnit->Del(service);
        }
        return XCode::Successful;
    }
	XCode LocationService::Register(const s2s::cluster::server& request, s2s::cluster::list& response)
	{
		const std::string & rpc = request.rpc();
		const std::string & http = request.http();
		const std::string & name = request.name();
		this->mLocationComponent->AddRpcServer(name, rpc);
		this->mLocationComponent->AddHttpServer(name, http);
		InnerService * innerService = this->GetComponent<InnerService>();
		InnerNetComponent * innerNetComponent = this->GetComponent<InnerNetComponent>();
		if(innerNetComponent != nullptr)
		{
			std::vector<const ServiceNodeInfo *> services;
			innerNetComponent->GetServiceList(services);
			for(const ServiceNodeInfo * nodeInfo : services)
			{
				s2s::cluster::server * server = response.add_list();
				{
					server->set_name(nodeInfo->SrvName);
					server->set_rpc(nodeInfo->LocationRpc);
					server->set_http(nodeInfo->LocationHttp);
				}
				const std::string & address = nodeInfo->LocationRpc;
				innerService->Call(address, "Join", request);
			}
			const ServerConfig * config = ServerConfig::Inst();
			s2s::cluster::server * localServer = response.add_list();
			localServer->set_name(config->Name());
			config->GetLocation("rpc", *localServer->mutable_rpc());
			config->GetLocation("http", *localServer->mutable_http());
		}
		return XCode::Successful;
	}
}