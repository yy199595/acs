//
// Created by yjz on 2022/4/29.
//

#include "RedisSubComponent.h"
#include "MainRedisComponent.h"
namespace Sentry
{
	bool RedisSubComponent::LateAwake()
	{
		this->mTaskComponent = this->GetComponent<TaskComponent>();
		this->mMainRedisComponent = this->GetComponent<MainRedisComponent>();
		this->mConfig = this->GetApp()->GetConfig().GetRedisConfig("main");
		return true;
	}
	XCode RedisSubComponent::OnRequest(std::shared_ptr<Rpc_Request> request)
	{
		return XCode::CommandArgsError;
	}
	XCode RedisSubComponent::OnResponse(std::shared_ptr<Rpc_Response> response)
	{
		return XCode::CommandArgsError;
	}
}