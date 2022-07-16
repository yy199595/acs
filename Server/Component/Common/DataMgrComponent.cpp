//
// Created by yjz on 2022/4/16.
//

#include "DataMgrComponent.h"
#include"Component/Redis/MainRedisComponent.h"
#include"Component/Mysql/MysqlDataComponent.h"
namespace Sentry
{
	bool DataMgrComponent::LateAwake()
	{
		this->mRedisComponent = this->GetComponent<MainRedisComponent>();
		this->mMysqlComponent = this->GetComponent<MysqlDataComponent>();
		return true;
	}

	XCode DataMgrComponent::Set(long long key, const Message & message)
	{
		return XCode::Successful;
	}

	XCode DataMgrComponent::Set(const std::string& key, const Message & message)
	{
		return XCode::Successful;
	}

	XCode DataMgrComponent::Get(long long key, std::shared_ptr<Message> result)
	{
		return XCode::Successful;
	}

	XCode DataMgrComponent::Get(const std::string& key, std::shared_ptr<Message> result)
	{
		return XCode::Successful;
	}

	XCode DataMgrComponent::Add(long long key, const Message& message)
	{

		return XCode::Successful;
	}

	XCode DataMgrComponent::Add(const std::string& key, const Message& message)
	{

		return XCode::Successful;
	}
}