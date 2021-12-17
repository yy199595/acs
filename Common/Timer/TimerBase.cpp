#include "TimerBase.h"

namespace GameKeeper
{
    TimerBase::TimerBase(long long ms)
    {
        this->mDelayTime = ms;
        this->mTriggerTime = Helper::Time::GetMilTimestamp() + ms;
    }
}// namespace GameKeeper