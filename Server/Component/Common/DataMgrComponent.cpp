//
// Created by yjz on 2022/4/16.
//

#include"DataMgrComponent.h"
#include"Util/ProtoHelper.h"
#include"Component/Redis/RedisDataComponent.h"
#include"Component/Mysql/MysqlDataComponent.h"
#include"Component/Mongo/MongoAgentComponent.h"
#include"Component/Mongo/DataSyncRedisComponent.h"
#include"Component/Scene/ProtocolComponent.h"
namespace Sentry
{
	bool DataMgrComponent::LateAwake()
    {
        this->mRedisComponent = this->GetComponent<RedisDataComponent>();
        this->mMongoComponent = this->GetComponent<MongoAgentComponent>();
        this->mSyncComponent = this->GetComponent<DataSyncRedisComponent>();
        this->mProtoComponent = this->GetComponent<ProtocolComponent>();
        return this->mRedisComponent != nullptr && this->mMongoComponent != nullptr;
    }

    Pool::DataPool<long long, Message> * DataMgrComponent::GetPool1(const std::string &name)
    {
        auto iter = this->mNumberMap.find(name);
        if(iter == this->mNumberMap.end())
        {
            Pool::DataPool<long long, Message> * pool =
                    new Pool::DataPool<long long, Message>(10000);
            this->mNumberMap.emplace(name, pool);
            return pool;
        }
        return iter->second;
    }

    Pool::DataPool<std::string, Message> * DataMgrComponent::GetPool2(const std::string &name)
    {
        auto iter = this->mStringMap.find(name);
        if(iter == this->mStringMap.end())
        {
            Pool::DataPool<std::string, Message> * pool =
                    new Pool::DataPool<std::string, Message>(10000);
            this->mStringMap.emplace(name, pool);
            return pool;
        }
        return iter->second;
    }

	XCode DataMgrComponent::Set(long long id, std::shared_ptr<Message> message)
	{
        const std::string name = message->GetTypeName();
        NumberMessagePool * dataPool = this->GetPool1(name);
        if(dataPool != nullptr)
        {
            dataPool->Add(id, message);
        }
        size_t pos = name.find('_');
        if(pos != std::string::npos)
        {
            const std::string db = name.substr(0, pos);
            const std::string key = name.substr(pos + 1);
            this->mRedisComponent->RunCommand(db, "HDEL", key, id);
        }
        if(this->mMongoComponent->Save(*message) != XCode::Successful)
        {
            dataPool->Remove(id);
            return XCode::Failure;
        }
        return XCode::Successful;
	}

    XCode DataMgrComponent::Set(const std::string &id, std::shared_ptr<Message> message)
    {
        const std::string name = message->GetTypeName();
        StringMessagePool * dataPool = this->GetPool2(name);
        if(dataPool != nullptr)
        {
            dataPool->Add(id, message);
        }
        size_t pos = name.find('_');
        if(pos != std::string::npos)
        {
            const std::string db = name.substr(0, pos);
            const std::string key = name.substr(pos + 1);
            this->mRedisComponent->RunCommand(db, "HDEL", key, id);
        }
        if(this->mMongoComponent->Save(*message) != XCode::Successful)
        {
            dataPool->Remove(id);
            return XCode::Failure;
        }
        return XCode::Successful;
    }

	std::shared_ptr<Message> DataMgrComponent::Get(long long id, const std::string &name)
	{
        NumberMessagePool * dataPool = this->GetPool1(name);
        std::shared_ptr<Message> result = dataPool->Get(id);
        if(result != nullptr)
        {
            return result;
        }
        result = this->mProtoComponent->New(name);
        if(result == nullptr)
        {
            return nullptr;
        }
        size_t pos = name.find('_');
        if(pos != std::string::npos)
        {
            const std::string db = name.substr(0, pos);
            const std::string key = name.substr(pos + 1);
            std::shared_ptr<RedisResponse> response =
                    this->mRedisComponent->RunCommand(db, "HGET", key, id);
            if(response != nullptr && (response->GetType() == RedisRespType::REDIS_STRING
                || response->GetType() == RedisRespType::REDIS_BIN_STRING))
            {
                RedisString * redisString = (RedisString*)response->Get(0);
                if(util::JsonStringToMessage(redisString->GetValue(), result.get()).ok())
                {
                    dataPool->Add(id, result);
                    return result;
                }
            }
            const std::string select = fmt::format("{_id:{0}}", id);
            XCode code = this->mMongoComponent->Query(name.c_str(), select, result);
            if(code == XCode::Successful)
            {
                std::string json;
                dataPool->Add(id, result);
                util::MessageToJsonString(*result, &json);
                this->mRedisComponent->RunCommand(db, "HSET", key, id, json);
                return result;
            }
            dataPool->Remove(id);
            return nullptr;
        }
        const std::string select = fmt::format("{_id:{0}}", id);
        XCode code = this->mMongoComponent->Query(name.c_str(), select, result);
        if(code != XCode::Successful)
        {
            dataPool->Remove(id);
            return nullptr;
        }
        dataPool->Add(id, result);
        return result;
	}

    std::shared_ptr<Message> DataMgrComponent::Get(const std::string& id, const std::string & name)
	{
        StringMessagePool * dataPool = this->GetPool2(name);
        std::shared_ptr<Message> result = dataPool->Get(id);
        if(result != nullptr)
        {
            return result;
        }
        result = this->mProtoComponent->New(name);
        if(result == nullptr)
        {
            return nullptr;
        }
        size_t pos = name.find('_');
        if(pos != std::string::npos)
        {
            const std::string db = name.substr(0, pos);
            const std::string key = name.substr(pos + 1);
            std::shared_ptr<RedisResponse> response =
                    this->mRedisComponent->RunCommand(db, "HGET", key, id);
            if(response != nullptr && (response->GetType() == RedisRespType::REDIS_STRING
                                       || response->GetType() == RedisRespType::REDIS_BIN_STRING))
            {
                RedisString * redisString = (RedisString*)response->Get(0);
                if(util::JsonStringToMessage(redisString->GetValue(), result.get()).ok())
                {
                    dataPool->Add(id, result);
                    return result;
                }
            }
            const std::string select = fmt::format("{_id:\"{0}\"}", id);
            XCode code = this->mMongoComponent->Query(name.c_str(), select, result);
            if(code == XCode::Successful)
            {
                std::string json;
                dataPool->Add(id, result);
                util::MessageToJsonString(*result, &json);
                this->mRedisComponent->RunCommand(db, "HSET", key, id, json);
                return result;
            }
            dataPool->Remove(id);
            return nullptr;
        }
        const std::string select = fmt::format("{_id:{0}}", id);
        XCode code = this->mMongoComponent->Query(name.c_str(), select, result);
        if(code != XCode::Successful)
        {
            dataPool->Remove(id);
            return nullptr;
        }
        dataPool->Add(id, result);
        return result;
	}
}