#include"ActionTimeoutTimer.h"
#include<NetWork/NetWorkRetAction.h>
#include<Manager/ActionManager.h>
#include<Pool/ObjectPool.h>
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
		NetWorkPacket * packet = NetPacketPool.Create();
		packet->set_rpcid(this->mCallbackId);
		packet->set_code(XCode::TimeoutAutoCall);
		this->mActionManager->InvokeCallback(mCallbackId, packet);
		NetPacketPool.Destory(packet);
		return false;
	}
}