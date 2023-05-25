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
		BIND_COMMON_RPC_METHOD(ActorRegistry::Query);
		BIND_COMMON_RPC_METHOD(ActorRegistry::Register);
		this->mRedisComponent = this->GetComponent<RedisComponent>();
		return true;
	}

	int ActorRegistry::Query(const s2s::registry::query& request)
	{
		const std::string & name = request.name();
		auto iter = this->mRegistrys.find(name);
		if(iter != this->mRegistrys.end())
		{
			return XCode::Failure;
		}
		return XCode::Successful;
	}

	int ActorRegistry::Register(const s2s::registry::request& request)
	{
		long long actorId = request.id();
		const std::string & name = request.name();
		auto iter = this->mRegistrys.find(name);
		if(iter == this->mRegistrys.end())
		{
			std::unordered_map<long long, std::string> map;
			this->mRegistrys.emplace(name, map);
		}
		std::shared_ptr<RedisResponse> response =
			this->mRedisComponent->Run("HSET", name, actorId, request.json());
		if(response == nullptr || response->HasError())
		{
			return XCode::SaveToRedisFailure;
		}
		this->mRegistrys[name][actorId] = request.json();
		return XCode::Successful;
	}
}