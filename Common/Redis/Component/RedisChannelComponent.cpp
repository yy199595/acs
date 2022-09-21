//
// Created by mac on 2022/5/25.
//

#include"RedisChannelComponent.h"
#include"Component/RedisSubComponent.h"
namespace Sentry
{
	bool RedisChannelComponent::StartRegisterEvent()
	{
		return this->OnRegisterEvent(this->mEventRegistry);
	}

	bool RedisChannelComponent::Invoke(const std::string& id, std::shared_ptr<Json::Reader> json)
	{
		std::shared_ptr<EventMethod> eventMethod = this->mEventRegistry.GetEvent(id);
		return eventMethod != nullptr && eventMethod->Run(json);
	}
}