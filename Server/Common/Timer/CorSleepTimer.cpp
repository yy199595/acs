#include"CorSleepTimer.h"
#include<Util/TimeHelper.h>
namespace SoEasy
{
	CorSleepTimer::CorSleepTimer(CoroutineManager * sheduler, long long id, long long ms)
		: TimerBase(ms)
	{
		this->mCoroutineId = id;
		this->mScheduler = sheduler;
	}

	bool CorSleepTimer::Invoke()
	{
		this->mScheduler->Resume(this->mCoroutineId);
		return true;
	}
}
