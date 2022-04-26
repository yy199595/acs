//
// Created by yjz on 2022/4/16.
//

#include "DataMgrComponent.h"
#include"Component/Redis/MainRedisComponent.h"
#include"Component/Mysql/MysqlProxyComponent.h"
namespace Sentry
{
	bool DataMgrComponent::LateAwake()
	{
		this->mRedisComponent = this->GetComponent<MainRedisComponent>();
		this->mMysqlComponent = this->GetComponent<MysqlProxyComponent>();
		return true;
	}

	XCode DataMgrComponent::Set(long long key, const Message & message)
	{
		std::string data;
		if(!util::MessageToJsonString(message, &data).ok())
		{
			return XCode::JsonCastProtoFailure;
		}
		if(this->mMysqlComponent->Save(message) != XCode::Successful)
		{
			return XCode::SaveToMysqlFailure;
		}
		const std::string tab = message.GetTypeName();
		std::shared_ptr<RedisResponse> response = this->mRedisComponent->InvokeCommand("HSET", tab, key, data);
		if(response == nullptr || response->HasError())
		{
			return XCode::SaveToRedisFailure;
		}
		return XCode::Successful;
	}

	XCode DataMgrComponent::Set(const std::string& key, const Message & message)
	{
		std::string data;
		assert(key.size() <= 64);
		if(!util::MessageToJsonString(message, &data).ok())
		{
			return XCode::JsonCastProtoFailure;
		}
		if(this->mMysqlComponent->Save(message) != XCode::Successful)
		{
			return XCode::SaveToMysqlFailure;
		}
		const std::string tab = message.GetTypeName();
		std::shared_ptr<RedisResponse> response = this->mRedisComponent->InvokeCommand("HSET", tab, key, data);
		if(response == nullptr || response->HasError())
		{
			return XCode::SaveToRedisFailure;
		}
		return XCode::Successful;
	}

	XCode DataMgrComponent::Get(long long key, std::shared_ptr<Message> result)
	{
		const std::string tab = result->GetTypeName();
		std::shared_ptr<RedisResponse> response = this->mRedisComponent->InvokeCommand("HGET", tab, key);
		if(response != nullptr && response->GetArraySize() == 1)
		{
			const std::string & json = response->GetValue();
			if(util::JsonStringToMessage(json, result.get()).ok())
			{
				return XCode::Successful;
			}
		}
		Json::Writer jsonWriter;
		const Descriptor * descriptor = result->GetDescriptor();
		const FieldDescriptor *fileDesc = descriptor->FindFieldByNumber(1);
		jsonWriter.AddMember(fileDesc->name().c_str(), key);
		return this->mMysqlComponent->QueryOnce(jsonWriter.ToJsonString(), result);
	}

	XCode DataMgrComponent::Get(const std::string& key, std::shared_ptr<Message> result)
	{
		const std::string tab = result->GetTypeName();
		std::shared_ptr<RedisResponse> response = this->mRedisComponent->InvokeCommand("HGET", tab, key);
		if(response != nullptr && response->GetArraySize() == 1)
		{
			const std::string & json = response->GetValue();
			if(util::JsonStringToMessage(json, result.get()).ok())
			{
				return XCode::Successful;
			}
		}
		Json::Writer jsonWriter;
		const Descriptor * descriptor = result->GetDescriptor();
		const FieldDescriptor *fileDesc = descriptor->FindFieldByNumber(1);
		jsonWriter.AddMember(fileDesc->name().c_str(), key);
		return this->mMysqlComponent->QueryOnce(jsonWriter.ToJsonString(), result);
	}

	XCode DataMgrComponent::Add(long long key, const Message& message)
	{
		const std::string tab = message.GetTypeName();
		std::shared_ptr<RedisResponse> response = this->mRedisComponent->InvokeCommand("HSETNX", tab, key);
		if(response != nullptr && response->GetNumber() == 1)
		{
			return this->mMysqlComponent->Add(message);
		}
		return XCode::Failure;
	}

	XCode DataMgrComponent::Add(const std::string& key, const Message& message)
	{
		assert(key.size() <= 64);
		const std::string tab = message.GetTypeName();
		std::shared_ptr<RedisResponse> response = this->mRedisComponent->InvokeCommand("HSETNX", tab, key);
		if(response != nullptr && response->GetNumber() == 1)
		{
			return this->mMysqlComponent->Add(message);
		}
		return XCode::Failure;
	}

	void DataMgrComponent::OnSecondUpdate()
	{


	}
}