#pragma once

#include "TimerBase.h"
#include <Coroutine/CoroutineComponent.h>

namespace GameKeeper
{
    class CorSleepTimer : public TimerBase
    {
    public:
        CorSleepTimer(CoroutineComponent *sheduler, long long id, long long ms);

        ~CorSleepTimer() { this->mScheduler = nullptr; }

    public:
        bool Invoke() override;

    private:
        long long mCoroutineId;
        long long mNextInvokeTime;
        CoroutineComponent *mScheduler;
    };
}// namespace GameKeeper