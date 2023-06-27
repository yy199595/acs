//
// Created by yjz on 2023/5/25.
//

#include"ServerRegistry.h"
#include"Entity/Actor/App.h"
#include"Util/String/StringHelper.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Registry/Target/RedisRegistryComponent.h"
#include"Registry/Target/MongoRegistryComponent.h"
namespace Tendo
{
	ServerRegistry::ServerRegistry()
		: mTable("server"), mTarget(nullptr){ }

	bool ServerRegistry::Awake()
	{
		return this->mApp->AddComponent<MongoRegistryComponent>();
	}
	
	bool ServerRegistry::OnInit()
	{
		BIND_COMMON_RPC_METHOD(ServerRegistry::Add);
		BIND_COMMON_RPC_METHOD(ServerRegistry::Del);
		BIND_COMMON_RPC_METHOD(ServerRegistry::Query);
		BIND_COMMON_RPC_METHOD(ServerRegistry::Watch);
		this->mTarget = this->GetComponent<MongoRegistryComponent>();
		return true;
	}

	int ServerRegistry::Query(const registry::query::request& request, registry::query::response & response)
	{
		long long actorId = request.id();
		if(actorId == 0)
		{
			if(!request.name().empty())
			{
				const std::string & name = request.name();
				return this->mTarget->Query(this->mTable, name, response);
			}
			return this->mTarget->Query(this->mTable, response);
		}
		return this->mTarget->Query(this->mTable, actorId, response);
	}

	int ServerRegistry::Add(long long actorId, const com::type::json & request)
	{
		int id = 0;
		std::string name;
		Json::Reader document;
		const std::string& json = request.json();
		LOG_ERROR_CHECK_ARGS(document.ParseJson(json));
		LOG_ERROR_CHECK_ARGS(document.GetMember("id", id));
		LOG_ERROR_CHECK_ARGS(document.GetMember("name", name));
		const NodeConfig * nodeConfig = ClusterConfig::Inst()->GetConfig(name);
		if(nodeConfig == nullptr)
		{
			LOG_ERROR("not find [" << name << "] config");
			return XCode::CallArgsError;
		}
		int code = this->mTarget->Add(this->mTable, actorId, json);
		if(code == XCode::Successful)
		{
			LOG_DEBUG(name << " register successful : " << Helper::Str::FormatJson(json));
			return XCode::Successful;
		}
		LOG_ERROR(name << " register fail : " << Helper::Str::FormatJson(json));
		return code;
	}

	int ServerRegistry::Del(long long actorId)
	{
		return this->mTarget->Del(this->mTable, actorId);
	}

	int ServerRegistry::Watch(long long id, const com::array::string& request)
	{
		return XCode::Successful;
	}
}