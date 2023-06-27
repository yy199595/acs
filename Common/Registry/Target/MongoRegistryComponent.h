#pragma once
#include"TargetRegistryComponent.h"
#include "Mongo/Client/MongoClient.h"

namespace Tendo
{
	class MongoDBComponent;
	class MongoRegistryComponent : public TargetRegistryComponent
	{
	public:
		MongoRegistryComponent();
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
		int Encode(Bson::Reader::Document * document, registry::query::response &response);
	private:
		int mIndex;
		std::string mTable;
		MongoDBComponent* mMongo;
		Mongo::MongoConfig mConfig;
	};
}
