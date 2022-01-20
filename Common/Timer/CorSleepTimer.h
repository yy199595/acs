#pragma once

#include "TimerBase.h"
#include <Coroutine/TaskComponent.h>

namespace Sentry
{
    class CorSleepTimer : public TimerBase
    {
    public:
        CorSleepTimer(TaskComponent *sheduler, long long id, long long ms);

        ~CorSleepTimer() final { this->mScheduler = nullptr; }

    public:
        bool Invoke() override;

    private:
        long long mCoroutineId;
        long long mNextInvokeTime;
        TaskComponent *mScheduler;
    };
}// namespace Sentry