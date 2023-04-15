//
// Created by MyPC on 2023/4/15.
//

#include"RegistryComponent.h"
#include"Entity/Unit/App.h"
#include"Registry/Service/Registry.h"
#include"Redis/Component/RedisLuaComponent.h"
#include"Server/Config/ServerConfig.h"
#include"Server/Config/CodeConfig.h"
#include"Rpc/Component/NodeMgrComponent.h"
#include"Cluster/Config/ClusterConfig.h"

namespace Tendo
{
	RegistryComponent::RegistryComponent()
	{
		this->mRedisComponent = nullptr;
	}

	bool RegistryComponent::LateAwake()
	{
		const ServerConfig * config = ServerConfig::Inst();
		if(!config->GetMember("registry", this->mAddress))
		{
			LOG_ERROR("not find config registry address");
			return false;
		}
		this->mNodeComponent = this->GetComponent<NodeMgrComponent>();
		this->mRedisComponent = this->GetComponent<RedisLuaComponent>();
		return true;
	}

	void RegistryComponent::OnLocalComplete()
	{
		const std::string func("Register");
		const ServerConfig* config = ServerConfig::Inst();
		RpcService* rpcService = this->mApp->GetService<Registry>();

		s2s::server::info message;
		{
			message.set_id(config->GetId());
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
				this->Query(server, false);
				if(!this->mNodeComponent->HasServer(server))
				{
					LOG_WARN("------ wait [" << server << "] start ------");
					this->mApp->GetTaskComponent()->Sleep(1000 * 2);
				}
			}
		}
	}

	bool RegistryComponent::Query(const string& server, bool async)
	{
		const std::string func("registry.query");
		LOG_CHECK_RET_FALSE(this->mRedisComponent);

		std::string json;
		Json::Writer jsonWriter;
		jsonWriter.Add("name").Add(server);
		jsonWriter.WriterStream(&json);
		std::shared_ptr<RedisResponse> response
			= this->mRedisComponent->Call(func, json, async);
		LOG_CHECK_RET_FALSE(response && response->GetArraySize() > 0);
		for(size_t index = 0; index < response->GetArraySize(); index++)
		{
			const RedisString * redisString = response->Cast<RedisString>(index);
			if(redisString != nullptr)
			{
				Json::Reader jsonReader;
				const std::string & json = redisString->GetValue();
				if(!jsonReader.ParseJson(json))
				{
					return false;
				}
				int id = 0;
				std::string address;
				jsonReader.GetMember("id", id);
				const rapidjson::Value * jsonValue = jsonReader.GetJsonValue("listens");
				if(jsonValue != nullptr && jsonValue->IsObject())
				{
					auto iter = jsonValue->MemberBegin();
					for(; iter != jsonValue->MemberEnd(); iter++)
					{
						const char * key = iter->name.GetString();
						const char * address = iter->value.GetString();
						this->mNodeComponent->AddServer(id, server, key, address);
					}
				}
			}
		}
		return true;
	}

}