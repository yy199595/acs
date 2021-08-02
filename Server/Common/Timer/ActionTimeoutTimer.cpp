#include"ActionTimeoutTimer.h"
#include<NetWork/NetWorkRetAction.h>
#include<Manager/ActionManager.h>
#include<Pool/ObjectPool.h>
namespace Sentry
{
	ActionTimeoutTimer::ActionTimeoutTimer(long long ms, long long callbackId, ActionManager * mgr)
		:TimerBase(ms)
	{
		this->mActionManager = mgr;
		this->mCallbackId = callbackId;
	}

	bool ActionTimeoutTimer::Invoke()
	{
		// TODO
		return false;
	}
}