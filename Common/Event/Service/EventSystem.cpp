//
// Created by 64658 on 2024/9/2.
//

#include "EventSystem.h"
#include "Rpc/Component/DispatchComponent.h"
namespace acs
{
	EventSystem::EventSystem()
	{
		this->mDispatch = nullptr;
	}

	bool EventSystem::OnInit()
	{
		BIND_SERVER_RPC_METHOD(EventSystem::Publish);
		this->mDispatch = this->GetComponent<DispatchComponent>();
		return true;
	}

	int EventSystem::Publish(const rpc::Message & request)
	{
		std::string channel;
		const std::string & message = request.GetBody();
		LOG_ERROR_CHECK_ARGS(request.ConstHead().Get("channel", channel));
		std::unique_ptr<rpc::Message> eventMessage = std::make_unique<rpc::Message>();
		{
			eventMessage->SetType(rpc::Type::Request);
			eventMessage->SetContent(rpc::Porto::Json, message);
			eventMessage->GetHead().Add(rpc::Header::func, channel);
		}
		if(this->mDispatch->OnMessage(eventMessage.get()) != XCode::Ok)
		{
			return XCode::Ok;
		}
		eventMessage.release();
		return XCode::Ok;
	}
}