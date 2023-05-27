#include"MongoRegistry.h"
#include"Entity/Actor/App.h"
#include"Mongo/Component/MongoDBComponent.h"
namespace Tendo
{
	bool MongoRegistry::Awake(App* app)
	{
		app->AddComponent<MongoDBComponent>();
		return false;
	}

	bool MongoRegistry::LateAwake(App* app)
	{
		this->mMongo = app->GetComponent<MongoDBComponent>();
		return this->mMongo != nullptr;
	}

	int MongoRegistry::Del(const std::string& name, long long id)
	{
		return 0;
	}

	int MongoRegistry::Add(const std::string& name, long long id, const std::string& json)
	{
		return 0;
	}

	int MongoRegistry::Query(const std::string& name, registry::query::response& response)
	{
		return 0;
	}

	int MongoRegistry::Query(const std::string& name, long long id, registry::query::response& response)
	{
		return 0;
	}

}