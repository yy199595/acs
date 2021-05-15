#pragma once
#include"TimerBase.h"
#include<Coroutine/CoroutineManager.h>
namespace SoEasy
{
	class CorSleepTimer : public TimerBase
	{
	public:
		CorSleepTimer(CoroutineManager * sheduler, long long id, long long ms);
		~CorSleepTimer() { this->mScheduler = nullptr; }
	public:
		bool Invoke() override;
	private:
		long long mCoroutineId;
		long long mNextInvokeTime;
		CoroutineManager * mScheduler;
	};
}