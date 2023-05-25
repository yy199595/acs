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
		std::string address;
		const ServerConfig * config = ServerConfig::Inst();
		if(!config->GetMember("registry", address))
		{
			LOG_ERROR("not find config registry address");
			return false;
		}
		std::string path;
		RedisConfig configLoader;
		config->GetPath("db", path);
		if(!configLoader.LoadConfig(path))
		{
			return false;
		}
		Asio::Context & io = this->mApp->MainThread();
		RedisClientConfig redisConfig = configLoader.Config();
		std::shared_ptr<Tcp::SocketProxy> socket = std::make_shared<Tcp::SocketProxy>(io);
		this->mMainClient = std::make_shared<RedisTcpClient>(socket, redisConfig, nullptr);
		return this->mMainClient->AuthUser();
	}

	bool RegistryComponent::RegisterServer()
	{
		long long time = Helper::Time::NowSecTime();
		const ServerConfig* config = ServerConfig::Inst();
		std::shared_ptr<RedisRequest> request = RedisRequest::Make("HSET");
		{
			request->AddParameter(config->Name());
			request->AddParameter(config->ServerId());
		}
		Json::Writer jsonWriter;
		std::vector<std::string> listens;
		ServerConfig::Inst()->GetListen(listens);
		for(const std::string & name : listens)
		{
			std::string address;
			if(config->GetLocation(name.c_str(), address))
			{
				jsonWriter.Add(name).Add(address);
			}
		}
		jsonWriter.Add("time").Add(time);
		request->AddParameter(jsonWriter.JsonString());
		return this->mMainClient->SyncCommand(request) != nullptr;
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