#include "TimerBase.h"
namespace Tendo
{
    TimerBase::TimerBase(long long ms)
    {
		this->mTimerId = Helper::Guid::Create();
        this->mStartTime = Helper::Time::NowMilTime();
        this->mTargetTime = this->mStartTime + ms;
    }
}// namespace Sentry