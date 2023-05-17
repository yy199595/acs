//
// Created by MyPC on 2023/4/15.
//

#include"RegistryComponent.h"
#include"Entity/Actor/App.h"
#include"Redis/Component/RedisLuaComponent.h"
#include"Server/Config/ServerConfig.h"
#include"Server/Config/CodeConfig.h"
#include"Rpc/Component/LocationComponent.h"
#include"Cluster/Config/ClusterConfig.h"
#include "s2s/s2s.pb.h"
#include"Rpc/Service/RpcService.h"
#include "XCode/XCode.h"

namespace Tendo
{
	RegistryComponent::RegistryComponent()
	{
		this->mLocationComponent = nullptr;
	}

	bool RegistryComponent::LateAwake()
	{
		std::string address;
		const ServerConfig * config = ServerConfig::Inst();
		if(!config->GetMember("registry", address))
		{
			LOG_ERROR("not find config registry address");
			return false;
		}
		this->mActor = std::make_unique<Actor>(0, address);
		this->mLocationComponent = this->GetComponent<LocationComponent>();
		return this->mActor->LateAwake();
	}

	void RegistryComponent::Complete()
	{
		const std::string func("Registry.Register");
		const ServerConfig* config = ServerConfig::Inst();

		s2s::server::info message;
		{
			message.set_server_name(config->Name());
			message.set_group_id(config->GroupId());
			message.set_server_id(config->ServerId());
		}

		std::vector<std::string> listens;
		ServerConfig::Inst()->GetListen(listens);
		for (const std::string& name: listens)
		{
			std::string listenAddress;
			config->GetLocation(name.c_str(), listenAddress);
			message.mutable_listens()->insert({ name, listenAddress });
		}

#ifdef __DEBUG__
		LOG_INFO("start register to [" << this->mActor->GetAddr() << "]");
#endif
		do
		{
			int code = this->mActor->Call(func, message);
			if (code == XCode::Successful)
			{
				LOG_INFO("register to [" << this->mActor->GetAddr() << "] successful");
				break;
			}
			LOG_ERROR("register to [" << this->mActor->GetAddr() << "] "
									  << CodeConfig::Inst()->GetDesc(code));
			this->mApp->GetCoroutine()->Sleep(1000 * 5);
		}
		while(true);

		std::vector<std::string> servers;
		std::vector<RpcService *> components;
		this->mApp->GetComponents<RpcService>(components);
		for(RpcService * rpcService1 : components)
		{
			const std::string & server = rpcService1->GetServer();
			while(!this->mLocationComponent->HasServer(server))
			{
				this->Query(server);
				if(!this->mLocationComponent->HasServer(server))
				{
					LOG_WARN("------ wait [" << server << "] start ------");
					this->mApp->GetCoroutine()->Sleep(1000 * 2);
				}
			}
		}
	}

	int RegistryComponent::Query(const string& server)
	{
		s2s::server::query request;
		{
			request.set_group_id(0);
			request.set_server_name(server);
		}
		const std::string func("Registry.Query");
		std::shared_ptr<s2s::server::list> response(new s2s::server::list());
		if(this->mActor->Call(func, request, response) != XCode::Successful)
		{

		}

		for(int index = 0; index < response->list_size(); index++)
		{
			const s2s::server::info & info = response->list(index);
			{
				int id = info.server_id();
				const std::string & name = info.server_name();
				ServerUnit * locationUnit = this->mLocationComponent->GetOrCreateServer(id, name);
				{
					for (auto iter = info.listens().begin(); iter != info.listens().end(); iter++)
					{
						const std::string& listen = iter->first;
						const std::string& address = iter->second;
						locationUnit->Add(listen, address);
					}
				}
			}
		}
		return response->list_size();
	}
}