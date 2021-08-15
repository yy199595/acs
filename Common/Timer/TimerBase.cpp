#include "TimerBase.h"

namespace Sentry
{
    TimerBase::TimerBase(long long ms)
    {
        this->mDelayTime = ms;
        this->mTimerId = NumberHelper::Create();
        this->mTriggerTime = TimeHelper::GetMilTimestamp() + ms;
    }
}// namespace Sentry