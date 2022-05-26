//
// Created by mac on 2022/5/25.
//

#include"NetEventComponent.h"
#include"Component/Redis/MainRedisComponent.h"
namespace Sentry
{
	bool NetEventComponent::StartRegisterEvent()
	{
		this->mRedisComponent = this->GetComponent<MainRedisComponent>();
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
		if(json.AddMember("eveId", id) && json.WriterStream(content))
		{
			this->mRedisComponent->Publish(this->GetName(), content);
			return true;
		}
		return false;
	}

	bool NetEventComponent::PublishEvent(const string& id)
	{
		Json::Writer json;
		return this->PublishEvent(id, json);
	}
}