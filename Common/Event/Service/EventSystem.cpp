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

	int EventSystem::Publish(const rpc::Packet & request)
	{
		std::string channel;
		const std::string & message = request.GetBody();
		LOG_ERROR_CHECK_ARGS(request.ConstHead().Get("channel", channel));
		rpc::Packet * eventMessage = new rpc::Packet();
		{
			eventMessage->SetType(rpc::Type::Request);
			eventMessage->GetHead().Add("func", channel);
			eventMessage->SetContent(rpc::Porto::Json, message);
		}
		if(this->mDispatch->OnMessage(eventMessage) != XCode::Ok)
		{
			delete eventMessage;
		}
		return XCode::Ok;
	}
}