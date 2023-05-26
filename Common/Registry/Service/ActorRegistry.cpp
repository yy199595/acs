//
// Created by yjz on 2023/5/25.
//

#include"ActorRegistry.h"
#include"Entity/Actor/App.h"
#include"Redis/Component/RedisComponent.h"
namespace Tendo
{
	bool ActorRegistry::Awake()
	{
		this->mApp->AddComponent<RedisComponent>();
		return true;
	}
	bool ActorRegistry::OnInit()
	{
		BIND_COMMON_RPC_METHOD(ActorRegistry::Add);
		BIND_COMMON_RPC_METHOD(ActorRegistry::Del);
		BIND_COMMON_RPC_METHOD(ActorRegistry::Query);
		this->mRedisComponent = this->GetComponent<RedisComponent>();
		return true;
	}

	int ActorRegistry::Query(const registry::query::request& request, registry::query::response & response)
	{
		long long actorId = request.actor_id();
		const std::string & name = request.name();
		if(actorId == 0)
		{
			return this->OnQuery(name, response);
		}
		return this->OnQuery(name, actorId, response);
	}

	int ActorRegistry::OnQuery(const std::string& name, registry::query::response& response)
	{

		return XCode::Successful;
	}

	int ActorRegistry::OnQuery(const std::string& name, long long id, registry::query::response& response)
	{
		static const std::string cmd("HGET");
		const std::string key = fmt::format("Registry.{0}", name);
		std::shared_ptr<RedisResponse> redisResponse = this->mRedisComponent->Run(cmd, key, id);
		if(redisResponse == nullptr || redisResponse->GetType() != RedisRespType::REDIS_STRING)
		{
			return XCode::Failure;
		}
		registry::actor * actor = response.add_actors();
		{
			actor->set_name(name);
			actor->set_actor_id(id);
			actor->set_actor_json(redisResponse->GetString());
		}
		return XCode::Successful;
	}

	int ActorRegistry::Add(const registry::actor & request)
	{
		static const std::string cmd("HSET");
		long long actorId = request.actor_id();
		const std::string & name = request.name();
		const std::string &	json = request.actor_json();
		const std::string key = fmt::format("Registry.{0}", name);

		std::shared_ptr<RedisResponse> response =
			this->mRedisComponent->Run(cmd, key, actorId, json);
		if(response == nullptr || response->HasError())
		{
			return XCode::SaveToRedisFailure;
		}
		return XCode::Successful;
	}

	int ActorRegistry::Del(const registry::actor& request)
	{
		static const std::string cmd("HDEL");
		long long actorId = request.actor_id();
		const std::string & name = request.name();
		const std::string key = fmt::format("Registry.{0}", name);

		std::shared_ptr<RedisResponse> response =
				this->mRedisComponent->Run(cmd, key, actorId);
		if(response == nullptr || response->HasError())
		{
			return XCode::SaveToRedisFailure;
		}
		return XCode::Successful;
	}
}