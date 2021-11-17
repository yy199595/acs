#include "TimerBase.h"

namespace GameKeeper
{
    TimerBase::TimerBase(long long ms)
    {
        this->mDelayTime = ms;
        this->mTriggerTime = TimeHelper::GetMilTimestamp() + ms;
    }
}// namespace GameKeeper