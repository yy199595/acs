#pragma once
#include<string>
#include"Message/s2s/registry.pb.h"
#include"Entity/Component/Component.h"
namespace Tendo
{
	class TargetRegistryComponent : public Component
	{
	public:
		virtual int Del(const std::string& name, long long id) = 0;
		virtual int Add(const std::string& name, long long id, const std::string& json) = 0;
	public:
		virtual int Query(const std::string& table, registry::query::response & response) = 0;
		virtual int Query(const std::string& table, long long id, registry::query::response& response) = 0;
		virtual int Query(const std::string& table, const std::string & name, registry::query::response & response) = 0;
	};
}