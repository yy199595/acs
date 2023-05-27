#pragma once
#include<string>
#include"Message/s2s/registry.pb.h"
namespace Tendo
{
	class App;
	class TargetRegistry
	{
	public:
		virtual bool Awake(App* app) = 0;
		virtual bool LateAwake(App* app) = 0;
		virtual int Del(const std::string& name, long long id) = 0;
		virtual int Add(const std::string& name, long long id, const std::string& json) = 0;
	public:
		virtual int Query(const std::string& name, registry::query::response & response) = 0;
		virtual int Query(const std::string& name, long long id, registry::query::response& response) = 0;
	};
}