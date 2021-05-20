#include"ActionTimeoutTimer.h"
#include<NetWork/NetWorkRetAction.h>
#include<Manager/ActionManager.h>
namespace SoEasy
{
	ActionTimeoutTimer::ActionTimeoutTimer(long long ms, long long callbackId, ActionManager * mgr)
		:TimerBase(ms)
	{
		this->mActionManager = mgr;
		this->mCallbackId = callbackId;
	}

	bool ActionTimeoutTimer::Invoke()
	{
		shared_ptr<LocalRetActionProxy> actionCallback = this->mActionManager->GetCallback(mCallbackId);
		if (actionCallback != nullptr)
		{
			std::shared_ptr<NetWorkPacket> packet = make_shared<NetWorkPacket>();
			packet->set_error_code(XCode::TimeoutAutoCall);
			actionCallback->Invoke(packet);
		}
		return false;
	}
}