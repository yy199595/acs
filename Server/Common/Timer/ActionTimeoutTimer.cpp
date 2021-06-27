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
		std::shared_ptr<NetWorkPacket> packet = make_shared<NetWorkPacket>();
		packet->set_rpcid(this->mCallbackId);
		packet->set_code(XCode::TimeoutAutoCall);
		this->mActionManager->AddActionArgv(this->mCallbackId, packet);
		return false;
	}
}