#include "CorSleepTimer.h"

namespace GameKeeper
{
    CorSleepTimer::CorSleepTimer(CoroutineComponent *sheduler, long long id, long long ms)
        : TimerBase(ms)
    {
        this->mCoroutineId = id;
        this->mScheduler = sheduler;
    }

    bool CorSleepTimer::Invoke()
    {
        assert(this->mScheduler);
        this->mScheduler->Resume(this->mCoroutineId);
        return true;
    }
}// namespace GameKeeper
