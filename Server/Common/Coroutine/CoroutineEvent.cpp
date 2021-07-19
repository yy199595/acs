#include"CoroutineEvent.h"
#include"CoroutineManager.h"
namespace Sentry
{
	CorSleepEvent::CorSleepEvent(CoroutineManager * scheduler, long long id, long long ms)
		: CoroutineEvent(scheduler, id)
	{
		this->mCorScheduler = scheduler;
		this->mNextInvokeTime = this->mCorScheduler->GetNowTime() + ms;
	}

	bool CorSleepEvent::Invoke()
	{
		long long nowTime = this->mCorScheduler->GetNowTime();
		return nowTime >= this->mNextInvokeTime;
	}

	CorNextFrameEvent::CorNextFrameEvent(CoroutineManager * scheduler, long long id, int count)
		: CoroutineEvent(scheduler, id)
	{
		this->mCount = 0;
		this->mMaxCount = count;
	}

	bool CorNextFrameEvent::Invoke()
	{
		this->mCount++;
		return this->mCount >= this->mMaxCount;
	}

	CoroutineEvent::CoroutineEvent(CoroutineManager * mgr, long long id)
		:mCorScheduler(mgr), mBindCoroutineId(id)
	{

	}
}