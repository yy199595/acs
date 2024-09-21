#include "TimerBase.h"
namespace acs
{
    TimerBase::TimerBase(long long ms)
    {
		this->mTimerId = help::Guid::Create();
        this->mTargetTime = help::Time::NowMil() + ms;
    }
}// namespace Sentry