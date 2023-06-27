#pragma once
#include"TargetRegistryComponent.h"
namespace Tendo
{
	class RedisComponent;
	class RedisRegistryComponent : public TargetRegistryComponent
	{
	public:
		RedisRegistryComponent();
	protected:
		bool Awake() final;
		bool LateAwake() final;
	protected:
		int Del(const std::string& name, long long id) final;
		int Add(const std::string& name, long long id, const std::string& json) final;
	protected:
		int Query(const std::string& name, registry::query::response& response) final;
		int Query(const std::string& name, long long id, registry::query::response& response) final;
		int Query(const std::string &table, const std::string &name, registry::query::response &response) final;
	private:
		RedisComponent* mRedis;
	};
}