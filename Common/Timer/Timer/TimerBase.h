#pragma once

#include"Guid/Guid.h"
#include<functional>
#include"Time/TimeHelper.h"

namespace Sentry
{
    class TimerBase
    {
    public:
        friend class TimerComponent;
        explicit TimerBase(long long ms);

        virtual ~TimerBase() = default;

    public:
        virtual void Invoke() = 0;
    public:
        long long GetTimerId() const { return mTimerId; }
        long long GetTargetTime() const { return this->mTargetTime; }
    private:
        long long mTimerId;
        long long mStartTime;
        long long mTargetTime;
    };
}// namespace Sentry