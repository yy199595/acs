//
// Created by yjz on 2022/4/16.
//

#include "DataMgrComponent.h"
#include"Component/Redis/RedisComponent.h"
#include"Component/Mysql/MysqlProxyComponent.h"
namespace Sentry
{
	bool DataMgrComponent::LateAwake()
	{
		this->mRedisComponent = this->GetComponent<RedisComponent>();
		this->mMysqlComponent = this->GetComponent<MysqlProxyComponent>();
		return true;
	}

	XCode DataMgrComponent::Set(const std::string& tab, long long key, const Message & data)
	{
		return XCode::Successful;
	}

	XCode DataMgrComponent::Set(const std::string& tab, const std::string& key, const Message & data)
	{
		return XCode::Successful;
	}

	XCode DataMgrComponent::Get(const std::string& tab, long long key, std::shared_ptr<Message> result)
	{
		return XCode::Successful;
	}

	XCode DataMgrComponent::Get(const std::string& tab, const std::string& key, std::shared_ptr<Message> result)
	{
		return XCode::Successful;
	}

	void DataMgrComponent::OnSecondUpdate()
	{


	}
}