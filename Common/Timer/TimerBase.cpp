#include "TimerBase.h"

namespace GameKeeper
{
    TimerBase::TimerBase(long long ms)
    {
        this->mDelayTime = ms;
        this->mTimerId = NumberHelper::Create();
        this->mTriggerTime = TimeHelper::GetMilTimestamp() + ms;
    }
}// namespace GameKeeper