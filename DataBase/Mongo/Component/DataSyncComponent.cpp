//
// Created by yjz on 2022/8/28.
//
#ifdef __ENABLE_REDIS__
#include"DataSyncComponent.h"
#include"String/StringHelper.h"
#include"Component/RedisComponent.h"
namespace Sentry
{
	bool DataSyncComponent::LateAwake()
	{
		this->mRedisComponent = this->GetComponent<RedisComponent>();
		return this->mRedisComponent != nullptr;
	}

    void DataSyncComponent::Set(const std::string &id, const std::string &fullName, const std::string &json)
    {
        size_t pos = fullName.find('.');
        if (pos == std::string::npos)
        {
            return;
        }
        const std::string db = fullName.substr(0, pos);
        const std::string tab = fullName.substr(pos + 1);
        this->Set(id, db, tab, json);
    }

	void DataSyncComponent::Set(const std::string& _id, const std::string & db, const std::string& tab, const std::string& json)
    {
        TcpRedisClient * redisClient = this->mRedisComponent->GetClient(db);
        if (redisClient == nullptr)
        {
            return;
        }
        std::shared_ptr<RedisResponse> response =
            this->mRedisComponent->Run(db, "HSET", tab, _id, json);
        CONSOLE_LOG_INFO("sync data to redis tab = " << tab << " json = " << json);
        if (response != nullptr && response->HasError())
        {
            CONSOLE_LOG_ERROR(response->GetString());
        }
    }

    void DataSyncComponent::Del(const std::string &id, const std::string &fullName)
    {
        size_t pos = fullName.find('.');
        if (pos == std::string::npos)
        {
            return;
        }
        const std::string db = fullName.substr(0, pos);
        const std::string tab = fullName.substr(pos + 1);
        this->Del(id, db, tab);
    }

    void DataSyncComponent::Del(const std::string &_id, const std::string & db, const std::string &tab)
    {
        TcpRedisClient * redisClient = this->mRedisComponent->GetClient(db);
        if (redisClient == nullptr)
        {
            return;
        }
        std::shared_ptr<RedisResponse> response =
            this->mRedisComponent->Run(redisClient, "HDEL", tab, _id);
        CONSOLE_LOG_INFO("remove data to redis tab = " << tab << " id = " << _id);
        if(response != nullptr && response->HasError())
        {
            CONSOLE_LOG_ERROR(response->GetString());
        }
    }
}
#endif