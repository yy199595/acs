#include"RedisRegistryComponent.h"
#include"XCode/XCode.h"
#include"Entity/Actor/App.h"
#include"Redis/Component/RedisComponent.h"
namespace Tendo
{
	RedisRegistryComponent::RedisRegistryComponent()
	{
		this->mRedis = nullptr;
	}

	bool RedisRegistryComponent::Awake()
	{
		this->mApp->AddComponent<RedisComponent>();
		return true;
	}
	
	bool RedisRegistryComponent::LateAwake()
	{
		this->mRedis = this->mApp->GetComponent<RedisComponent>();
		return this->mRedis != nullptr;
	}

	int RedisRegistryComponent::Del(const std::string& name, long long id)
	{
		static const std::string cmd("HDEL");
		const std::string key = fmt::format("Registry.{0}", name);
		std::shared_ptr<RedisResponse> response = this->mRedis->SyncRun(cmd, key, id);
		if (response == nullptr || response->HasError())
		{
			return XCode::SaveToRedisFailure;
		}
		return XCode::Successful;
	}

	int RedisRegistryComponent::Add(const std::string& name, long long id, const std::string& json)
	{
		static const std::string cmd("HSET");
		const std::string key = fmt::format("Registry.{0}", name);
		std::shared_ptr<RedisResponse> response = this->mRedis->SyncRun(cmd, key, id, json);
		if (response == nullptr || response->HasError())
		{
			return XCode::SaveToRedisFailure;
		}
		return XCode::Successful;
	}

	int RedisRegistryComponent::Query(const std::string& name, registry::query::response& response)
	{
		return XCode::Successful;
	}

	int RedisRegistryComponent::Query(const std::string& name, long long id, registry::query::response& response)
	{
		static const std::string cmd("HGET");
		const std::string key = fmt::format("Registry.{0}", name);
		std::shared_ptr<RedisResponse> redisResponse = this->mRedis->SyncRun(cmd, key, id);
		if (redisResponse == nullptr || redisResponse->GetType() != RedisRespType::REDIS_STRING)
		{
			return XCode::Failure;
		}
		response.add_actors(redisResponse->GetString());
		return XCode::Successful;
	}

	int RedisRegistryComponent::Query(const string& table, const string& name, registry::query::response& response)
	{
		return XCode::Successful;
	}
}