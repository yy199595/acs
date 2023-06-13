#include"RedisRegistry.h"
#include"XCode/XCode.h"
#include"Entity/Actor/App.h"
#include"Redis/Component/RedisComponent.h"
namespace Tendo
{
	RedisRegistry::RedisRegistry()
	{
		this->mRedis = nullptr;
	}

	bool RedisRegistry::Awake()
	{
		this->mApp->AddComponent<RedisComponent>();
		return true;
	}
	
	bool RedisRegistry::LateAwake()
	{
		this->mRedis = this->mApp->GetComponent<RedisComponent>();
		return this->mRedis != nullptr;
	}

	int RedisRegistry::Del(const std::string& name, long long id)
	{
		static const std::string cmd("HDEL");
		const std::string key = fmt::format("Registry.{0}", name);
		std::shared_ptr<RedisResponse> response = this->mRedis->Run(cmd, key, id);
		if (response == nullptr || response->HasError())
		{
			return XCode::SaveToRedisFailure;
		}
		return XCode::Successful;
	}

	int RedisRegistry::Add(const std::string& name, long long id, const std::string& json)
	{
		static const std::string cmd("HSET");
		const std::string key = fmt::format("Registry.{0}", name);
		std::shared_ptr<RedisResponse> response = this->mRedis->Run(cmd, key, id, json);
		if (response == nullptr || response->HasError())
		{
			return XCode::SaveToRedisFailure;
		}
		return XCode::Successful;
	}

	int RedisRegistry::Query(const std::string& name, registry::query::response& response)
	{
		return XCode::Successful;
	}

	int RedisRegistry::Query(const std::string& name, long long id, registry::query::response& response)
	{
		static const std::string cmd("HGET");
		const std::string key = fmt::format("Registry.{0}", name);
		std::shared_ptr<RedisResponse> redisResponse = this->mRedis->Run(cmd, key, id);
		if (redisResponse == nullptr || redisResponse->GetType() != RedisRespType::REDIS_STRING)
		{
			return XCode::Failure;
		}
		registry::actor* actor = response.add_actors();
		{
			actor->set_name(name);
			actor->set_actor_id(id);
			actor->set_actor_json(redisResponse->GetString());
		}
		return XCode::Successful;
	}
}