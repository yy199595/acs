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

namespace Tendo
{
	bool RegistryComponent::LateAwake()
	{
		std::string address;
		const ServerConfig * config = ServerConfig::Inst();
		if(!config->GetMember("registry", address))
		{
			LOG_ERROR("not find config registry address");
			return false;
		}
		this->mActor = std::make_shared<Server>(1, "RegistryServer");
		this->mActor->AddListen("rpc", address);
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
		for(const std::string & name : listens)
		{
			std::string address;
			config->GetLocation(name.c_str(), address);
			message.mutable_listens()->insert({name, address});
		}

		do
		{
			int code = this->mActor->Call(func, message);
			if (code == XCode::Successful)
			{
				LOG_INFO("register successful");
				break;
			}
			this->mApp->GetCoroutine()->Sleep(1000 * 5);
		}
		while(true);

		std::vector<std::string> servers;
		std::vector<RpcService *> components;
		this->mApp->GetComponents<RpcService>(components);
		for(RpcService * rpcService1 : components)
		{
			const std::string & server = rpcService1->GetServer();
			if(this->mApp->Random(server) == nullptr)
			{
				this->Query(server);
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
				if(this->mApp->ActorMgr()->GetActor(id) == nullptr)
				{
					std::shared_ptr<Server> actor = std::make_shared<Server>(id, name);
					{
						auto iter = info.listens().begin();
						for (; iter != info.listens().end(); iter++)
						{
							actor->AddListen(iter->first, iter->second);
						}
						this->mApp->ActorMgr()->AddServer(actor);
					}
				}
			}
		}
		return response->list_size();
	}
}