//
// Created by yjz on 2022/5/15.
//

#include"UserSyncComponent.h"
#include"Component/Redis/RedisDataComponent.h"
#include"Component/RpcService/LocalService.h"
namespace Sentry
{
	bool UserSyncComponent::LateAwake()
	{
		this->mRedisComponent = this->GetComponent<RedisDataComponent>();
		return true;
	}

	bool UserSyncComponent::OnRegisterEvent(NetEventRegistry& eventRegister)
	{
		return eventRegister.Sub("user_join_event", &UserSyncComponent::OnUserJoin, this)
			   && eventRegister.Sub("user_exit_event", &UserSyncComponent::OnUserExit, this);
	}

	bool UserSyncComponent::OnUserJoin(const Json::Reader& json)
	{
		std::string address;
		std::string service;
		long long userId = 0;
		LOG_CHECK_RET_FALSE(json.GetMember("user_id", userId));
		LOG_CHECK_RET_FALSE(json.GetMember("address", address));
		LOG_CHECK_RET_FALSE(json.GetMember("service", service));
		Service * component = this->GetComponent<Service>(service);
        return true;
	}

	bool UserSyncComponent::OnUserExit(const Json::Reader& json)
	{
		return true;
	}
}
