//
// Created by mac on 2022/5/25.
//

#include"NetEventComponent.h"
#include"Component/Redis/RedisSubComponent.h"
namespace Sentry
{
	bool NetEventComponent::StartRegisterEvent()
	{
		this->mRedisComponent = this->GetComponent<RedisSubComponent>();
		if(this->OnRegisterEvent(this->mEventRegistry))
		{
			if(this->mEventRegistry.GetEventSize() > 0)
			{
				return this->mRedisComponent->SubscribeChannel(this->GetName());
			}
		}
		return false;
	}

	bool NetEventComponent::Invoke(const std::string& id, std::shared_ptr<Json::Reader> json)
	{
		std::shared_ptr<EventMethod> eventMethod = this->mEventRegistry.GetEvent(id);
		return eventMethod != nullptr && eventMethod->Run(json);
	}

	bool NetEventComponent::PublishEvent(const std::string& id, Json::Writer& json)
	{
		std::string content;
		json << "eventId" << id;
		json.WriterStream(content);
		return this->mRedisComponent->Publish(this->GetName(), content) > 0;
	}

	bool NetEventComponent::PublishEvent(const string& id)
	{
		Json::Writer json;
		return this->PublishEvent(id, json);
	}
}