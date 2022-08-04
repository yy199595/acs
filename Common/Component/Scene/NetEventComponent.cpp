//
// Created by mac on 2022/5/25.
//

#include"NetEventComponent.h"
#include"Component/Redis/RedisSubComponent.h"
namespace Sentry
{
	bool NetEventComponent::StartRegisterEvent()
	{
		return this->OnRegisterEvent(this->mEventRegistry);
	}

	bool NetEventComponent::Invoke(const std::string& id, std::shared_ptr<Json::Reader> json)
	{
		std::shared_ptr<EventMethod> eventMethod = this->mEventRegistry.GetEvent(id);
		return eventMethod != nullptr && eventMethod->Run(json);
	}
}