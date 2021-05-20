#pragma once
#include"TimerBase.h"
namespace SoEasy
{
	class ActionManager;
	class ActionTimeoutTimer : public TimerBase
	{
	public:
		ActionTimeoutTimer(long long ms, long long callbackId, ActionManager * mgr);
	public:
		bool Invoke() final;
	private:
		long long mCallbackId;
		ActionManager * mActionManager;
	};
}