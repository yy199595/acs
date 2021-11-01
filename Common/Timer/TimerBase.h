#pragma once

#include <Util/NumberHelper.h>
#include <Util/TimeHelper.h>
#include <functional>

namespace GameKeeper
{
    class TimerBase
    {
    public:
        TimerBase(long long ms);

        virtual ~TimerBase() {}

    public:
        virtual bool Invoke() = 0;//
    public:
        const long long GetTimerId() { return mTimerId; }

        const long long GetTriggerTime() { return this->mTriggerTime; }

    protected:
        long long mTimerId;
        long long mDelayTime;
        long long mTriggerTime;
    };
}// namespace GameKeeper