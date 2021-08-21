#include"CoroutineEvent.h"
#include"CoroutineManager.h"
#include"Coroutine.h"
namespace Sentry
{
    CorSleepEvent::CorSleepEvent(CoroutineManager *scheduler, long long id, long long ms)
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

    CorNextFrameEvent::CorNextFrameEvent(CoroutineManager *scheduler, long long id, int count)
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

    CoroutineEvent::CoroutineEvent(CoroutineManager *mgr, long long id)
            : mCorScheduler(mgr), mBindCoroutineId(id)
    {

    }
	
}

namespace Sentry
{
	CoroutinePool::CoroutinePool(int count)
		:mId(0)
	{
		for (int index = 0; index < count; index++)
		{
			Coroutine *coroutine = new Coroutine();
			if (coroutine != nullptr)
			{
				coroutine->mCoroutineId = mId++;
				this->mAllCoroutine.push_back(coroutine);
				this->mIdleCoroutines.push(coroutine->mCoroutineId);
			}
		}
	}

	CoroutinePool::~CoroutinePool()
	{
		for (size_t index = 0; index < this->mAllCoroutine.size(); index++)
		{
			Coroutine *coroutine = this->mAllCoroutine[index];
			delete coroutine;
		}
		this->mAllCoroutine.clear();
	}
	Coroutine * CoroutinePool::Pop()
	{
		if (!this->mIdleCoroutines.empty())
		{
			unsigned int id = this->mIdleCoroutines.front();
			this->mIdleCoroutines.pop();
			return this->mAllCoroutine[id];
		}
		else
		{
			Coroutine *coroutine = new Coroutine();
			if (coroutine != nullptr)
			{
				coroutine->mCoroutineId = mId++;
				coroutine->mState = CorState::Ready;
				this->mAllCoroutine.push_back(coroutine);
			}
			return coroutine;
		}
	}
	void CoroutinePool::Push(Coroutine * coroutine)
	{
		if (coroutine != nullptr)
		{
			if (coroutine->mFunction != nullptr)
			{
				delete coroutine->mFunction;
				coroutine->mFunction = nullptr;
			}
			coroutine->mState = CorState::Finish;
			this->mIdleCoroutines.push(coroutine->mCoroutineId);
		}
	}
	Coroutine * CoroutinePool::Get(unsigned int id)
	{
		if (id > this->mAllCoroutine.size())
		{
			return nullptr;
		}
		return this->mAllCoroutine[id];	
	}
}