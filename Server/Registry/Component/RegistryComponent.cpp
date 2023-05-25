//
// Created by MyPC on 2023/4/15.
//

#include"RegistryComponent.h"
#include"Entity/Actor/App.h"
#include"Server/Config/ServerConfig.h"
#include"Server/Config/CodeConfig.h"
#include"Cluster/Config/ClusterConfig.h"
#include "s2s/s2s.pb.h"
#include"Rpc/Service/RpcService.h"
#include "XCode/XCode.h"
#include "Redis/Client/RedisTcpClient.h"
#include "Server/Component/ThreadComponent.h"
namespace Tendo
{
	bool RegistryComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mUnit->Cast<Server>());
		return this->mApp->ActorMgr()->GetServer(0) != nullptr;
	}

	bool RegistryComponent::RegisterServer()
	{
		const std::string func("Registry.Register");
		Server * server = this->mUnit->Cast<Server>();
		Server * registry = this->mApp->ActorMgr()->GetServer(0);

		s2s::registry::request request;
		request.set_name(server->Name());
		request.set_id(server->GetUnitId());
		server->OnRegister(*request.mutable_name());
		if(registry->Call(func, request) != XCode::Successful)
		{
			return false;
		}
		return true;
	}

	void RegistryComponent::Complete()
	{
		while(!this->RegisterServer())
		{
			LOG_ERROR("register " << this->mApp->Name() << " failure");
		}

	}

	int RegistryComponent::Query(const string& server)
	{
		return 0;
	}
}