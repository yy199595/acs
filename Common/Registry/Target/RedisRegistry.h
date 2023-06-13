#pragma once
#include"TargetRegistry.h"
namespace Tendo
{
	class RedisComponent;
	class RedisRegistry : public TargetRegistry
	{
	public:
		RedisRegistry();
	protected:
		bool Awake() final;
		bool LateAwake() final;
	protected:
		int Del(const std::string& name, long long id) final;
		int Add(const std::string& name, long long id, const std::string& json) final;
	protected:
		int Query(const std::string& name, registry::query::response& response) final;
		int Query(const std::string& name, long long id, registry::query::response& response) final;
	private:
		RedisComponent* mRedis;
	};
}