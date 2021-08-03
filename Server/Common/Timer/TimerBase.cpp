#include "TimerBase.h"
#include <iostream>

namespace Sentry
{
    TimerBase::TimerBase(long long ms)
    {
        this->mDelayTime = ms;
        this->mTimerId = NumberHelper::Create();
        this->mTriggerTime = TimeHelper::GetMilTimestamp() + ms;
    }
}