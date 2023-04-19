//
// Created by MyPC on 2023/4/15.
//

#include"RegistryComponent.h"
#include"Entity/Unit/App.h"
#include"Registry/Service/Registry.h"
#include"Redis/Component/RedisLuaComponent.h"
#include"Server/Config/ServerConfig.h"
#include"Server/Config/CodeConfig.h"
#include"Rpc/Component/LocationComponent.h"
#include"Cluster/Config/ClusterConfig.h"

namespace Tendo
{
	RegistryComponent::RegistryComponent()
	{
		this->mNodeComponent = nullptr;
	}

	bool RegistryComponent::LateAwake()
	{
		const ServerConfig * config = ServerConfig::Inst();
		if(!config->GetMember("registry", this->mAddress))
		{
			LOG_ERROR("not find config registry address");
			return false;
		}
		this->mNodeComponent = this->GetComponent<LocationComponent>();
		return true;
	}

	void RegistryComponent::OnLocalComplete()
	{
		const std::string func("Register");
		const ServerConfig* config = ServerConfig::Inst();
		RpcService* rpcService = this->mApp->GetService<Registry>();

		s2s::server::info message;
		{
			message.set_server_name(config->Name());
			message.set_group_id(config->GroupId());
			message.set_server_id(config->ServerId());
			std::vector<std::string> listens;
			config->GetListen(listens);
			for (const std::string& name: listens)
			{
				std::string listenAddress;
				config->GetLocation(name.c_str(), listenAddress);
				message.mutable_listens()->insert({ name, listenAddress });
			}
		}

#ifdef __DEBUG__
		LOG_INFO("start register to [" << this->mAddress << "]");
#endif
		do
		{
			int code = rpcService->Call(this->mAddress, func, message);
			if (code == XCode::Successful)
			{
				const std::string& server = rpcService->GetServer();
				LOG_INFO("register to [" << this->mAddress << "] successful");
				break;
			}
			LOG_ERROR("register to [" << this->mAddress << "] "
									  << CodeConfig::Inst()->GetDesc(code));
			this->mApp->GetTaskComponent()->Sleep(1000 * 5);
		}
		while(true);

		std::vector<std::string> servers;
		std::vector<RpcService *> components;
		this->mApp->GetComponents<RpcService>(components);
		for(RpcService * rpcService1 : components)
		{
			const std::string & server = rpcService1->GetServer();
			while(!this->mNodeComponent->HasServer(server))
			{
				if(this->Query(server) <= 0)
				{
					LOG_WARN("------ wait [" << server << "] start ------");
					this->mApp->GetTaskComponent()->Sleep(1000 * 2);
				}
			}
		}
	}

	int RegistryComponent::Query(const string& server)
	{
		com::type::string request;
		request.set_str(server);
		const std::string func("Query");
		RpcService * rpcService = this->mApp->GetService<Registry>();

		std::shared_ptr<s2s::server::list> response = std::make_shared<s2s::server::list>();
		if(rpcService->Call(this->mAddress, func, request, response) != XCode::Successful)
		{
			return 0;
		}
		for(int index = 0; index < response->list_size(); index++)
		{
			const s2s::server::info & info = response->list(index);
			{
				int id = info.server_id();
				const std::string & name = info.server_name();
				LocationUnit * locationUnit = this->mNodeComponent->GetOrCreateServer(id, name);
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