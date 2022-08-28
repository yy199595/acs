//
// Created by yjz on 2022/8/28.
//

#include"MongoSyncComponent.h"
#include"Util/StringHelper.h"
#include"Component/Redis/RedisDataComponent.h"
namespace Sentry
{
	bool MongoSyncComponent::LateAwake()
	{
		this->mRedisComponent = this->GetComponent<RedisDataComponent>();
		return this->mRedisComponent != nullptr;
	}
	void MongoSyncComponent::OnInsert(const s2s::mongo::index& data)
	{
		const std::string & tab = data.tab();
	}

	void MongoSyncComponent::OnQuery(long long _id, const std::string& tab, const std::string& json)
	{
		std::vector<std::string> result;
		if (Helper::String::Split(tab, "_", result) != 2)
		{
			return;
		}
		const std::string & key = result[1];
		const std::string & name = result[0];
		this->mRedisComponent->SenCommond(name, "HSET", key, _id, json);
	}

	void MongoSyncComponent::OnQuery(const std::string& _id, const std::string& tab, const std::string& json)
	{
		std::vector<std::string> result;
		if (Helper::String::Split(tab, "_", result) != 2)
		{
			return;
		}
		const std::string & key = result[1];
		const std::string & name = result[0];
		this->mRedisComponent->SenCommond(name, "HSET", key, _id, json);
	}
}