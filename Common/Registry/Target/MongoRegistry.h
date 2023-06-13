#pragma once
#include"TargetRegistry.h"
#include "Mongo/Client/MongoClient.h"

namespace Tendo
{
	class MongoDBComponent;
	class MongoRegistry : public TargetRegistry, public IStart
	{
	public:
		MongoRegistry();
	protected:
		void Start() final;
		bool Awake() final;
		bool LateAwake() final;
	protected:
		int Del(const std::string& name, long long id) final;
		int Add(const std::string& name, long long id, const std::string& json) final;
	protected:
		int Query(const std::string& name, registry::query::response& response) final;
		int Query(const std::string& name, long long id, registry::query::response& response) final;
	private:
		int mIndex;
		MongoDBComponent* mMongo;
		Mongo::MongoConfig mConfig;
	};
}
