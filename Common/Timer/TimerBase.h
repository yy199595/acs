#pragma once

#include <Util/Guid.h>
#include <Util/TimeHelper.h>
#include <functional>

namespace Sentry
{
	enum class TimerState
	{
		None,
		Ok,
		Cancel
	};


    class TimerBase
    {
    public:
        friend class TimerComponent;
        explicit TimerBase(long long ms);

        virtual ~TimerBase() = default;

    public:
        virtual void Invoke(TimerState state = TimerState::Ok) = 0;
    public:
        long long GetTimerId() const { return mTimerId; }
        long long GetTargetTime() const { return this->mTargetTime; }
    private:
        long long mTimerId;
        long long mStartTime;
        long long mTargetTime;
    };
}// namespace Sentry