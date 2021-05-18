#pragma once
#include"TimerBase.h"
namespace SoEasy
{
	class LocalActionManager;
	class ActionTimeoutTimer : public TimerBase
	{
	public:
		ActionTimeoutTimer(long long ms, long long callbackId, LocalActionManager * mgr);
	public:
		bool Invoke() final;
	private:
		long long mCallbackId;
		LocalActionManager * mActionManager;
	};
}