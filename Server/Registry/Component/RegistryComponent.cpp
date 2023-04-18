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
			message.set_name(config->Name());
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
		int code = XCode::Successful;
		do
		{
			code = rpcService->Call(this->mAddress, func, message);
			if (code != XCode::Successful)
			{
				LOG_ERROR("register to [" << this->mAddress << "] "
										  << CodeConfig::Inst()->GetDesc(code));
				this->mApp->GetTaskComponent()->Sleep(1000 * 5);
				continue;
			}
			const std::string& server = rpcService->GetServer();
			LOG_INFO("register to [" << this->mAddress << "] successful");
		}
		while(code != XCode::Successful);

		std::vector<std::string> servers;
		ClusterConfig::Inst()->GetServers(servers);
		for(const std::string & server : servers)
		{
			while(!this->mNodeComponent->HasServer(server))
			{
				if(this->Query(server) <= 0 && !this->mNodeComponent->HasServer(server))
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
				ServerData data(info.name());
				for (auto iter = info.listens().begin(); iter != info.listens().end(); iter++)
				{
					data.Add(iter->first, iter->second);
					const std::string& listen = iter->first;
					const std::string& address = iter->second;
				}
				this->mNodeComponent->AddServer(data);
			}
		}
		return response->list_size();
	}
}