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

        virtual ~TimerBase() = default;

    public:
        virtual bool Invoke() = 0;// true 表示完成 false 表示继续放进时间轮
    public:
        long long GetTimerId() const { return mTimerId; }

        long long GetTriggerTime() const { return this->mTriggerTime; }

    protected:
        long long mTimerId;
        long long mDelayTime;
        long long mTriggerTime;
    };
}// namespace GameKeeper