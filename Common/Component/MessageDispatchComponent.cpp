#include "MessageDispatchComponent.h"
#include <Core/App.h>
#include <Coroutine/CoroutineComponent.h>
#include <NetWork/PacketMapper.h>
#include <Service/ServiceComponent.h>
#include <NetWork/ServiceMethod.h>
namespace Sentry
{
	MessageDispatchComponent::MessageDispatchComponent()
		:mServiceObject(App::Get())
	{
		this->mIsStarted = false;
	}

	MessageDispatchComponent::~MessageDispatchComponent()
	{

	}

	bool MessageDispatchComponent::Awake()
	{
		this->mIsStarted = true;
		this->mCorComponent = App::Get().GetCoroutineComponent();
		this->mCorId = this->mCorComponent->StartCoroutine(&MessageDispatchComponent::HandleMessage, this);
		return true;
	}

	void MessageDispatchComponent::AddHandleMessage(PacketMapper * message)
	{
		this->mWaitMsgQueue.push(message);
		this->mCorComponent->Resume(mCorId);
	}

	void MessageDispatchComponent::HandleMessage()
	{
		while (this->mIsStarted)
		{
			if (this->mWaitMsgQueue.empty())
			{
				this->mCorComponent->YieldReturn();
			}
			PacketMapper * message = this->mWaitMsgQueue.front();
			this->mWaitMsgQueue.pop();
			const std::string & method = message->GetProConfig()->Method;
			const std::string & service = message->GetProConfig()->Service;
			ServiceComponent * serviceComponent = mServiceObject.GetComponent<ServiceComponent>(service);
			if (serviceComponent != nullptr && serviceComponent->HasMethod(method))
			{
				ServiceMethod * serviceMethod = serviceComponent->GetMethod(method);
				if (serviceMethod->IsLuaMethod()) {

				}
			}		
		}
	}

}
