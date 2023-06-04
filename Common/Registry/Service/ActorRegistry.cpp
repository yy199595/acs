//
// Created by yjz on 2023/5/25.
//

#include"ActorRegistry.h"
#include"Entity/Actor/App.h"
#include"Registry/Target/RedisRegistry.h"
#include"Registry/Target/MongoRegistry.h"
namespace Tendo
{
	bool ActorRegistry::Awake()
	{
		this->mTarget = std::make_unique<MongoRegistry>();
		return this->mTarget->Awake(this->mApp);
	}
	bool ActorRegistry::OnInit()
	{
		BIND_COMMON_RPC_METHOD(ActorRegistry::Add);
		BIND_COMMON_RPC_METHOD(ActorRegistry::Del);
		BIND_COMMON_RPC_METHOD(ActorRegistry::Query);
		return this->mTarget->LateAwake(this->mApp);
	}

	int ActorRegistry::Query(const registry::query::request& request, registry::query::response & response)
	{
		long long actorId = request.actor_id();
		const std::string & name = request.name();
		if(actorId == 0)
		{
			return this->mTarget->Query(name, response);
		}
		return this->mTarget->Query(name, actorId, response);
	}

	int ActorRegistry::Add(const registry::actor & request)
	{
		long long actorId = request.actor_id();
		const std::string & name = request.name();
		const std::string &	json = request.actor_json();
		return  this->mTarget->Add(name, actorId, json);
	}

	int ActorRegistry::Del(const registry::actor& request)
	{
		long long actorId = request.actor_id();
		const std::string & name = request.name();
		return this->mTarget->Del(name, actorId);
	}
}