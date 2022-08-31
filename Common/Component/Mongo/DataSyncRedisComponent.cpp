//
// Created by yjz on 2022/8/28.
//

#include"DataSyncRedisComponent.h"
#include"Util/StringHelper.h"
#include"Component/Redis/RedisDataComponent.h"
namespace Sentry
{
	bool DataSyncRedisComponent::LateAwake()
	{
		this->mRedisComponent = this->GetComponent<RedisDataComponent>();
		return this->mRedisComponent != nullptr;
	}

    void DataSyncRedisComponent::Set(const std::string &_id, const db::mongo::insert &data)
    {
        const std::string & tab = data.tab();
        this->Set(_id, tab, data.json());
    }

	void DataSyncRedisComponent::Set(const std::string& _id, const std::string& tab, const std::string& json)
	{
        size_t pos = tab.find('_');
        if(pos == std::string::npos)
        {
            return;
        }
        const std::string db = tab.substr(0, pos);
        const std::string key = tab.substr(pos + 1);
        std::shared_ptr<RedisResponse> response =
                this->mRedisComponent->RunCommand(db, "HSET", key, _id, json);
        if(response != nullptr && response->HasError())
        {
            CONSOLE_LOG_ERROR(response->GetString());
        }
	}

    void DataSyncRedisComponent::Del(const std::string &_id, const std::string &tab)
    {
        size_t pos = tab.find('_');
        if(pos == std::string::npos)
        {
            return;
        }
        const std::string db = tab.substr(0, pos);
        const std::string key = tab.substr(pos + 1);
        std::shared_ptr<RedisResponse> response =
                this->mRedisComponent->RunCommand(db, "HDEL", key, _id);
        if(response != nullptr && response->HasError())
        {
            CONSOLE_LOG_ERROR(response->GetString());
        }
    }
}