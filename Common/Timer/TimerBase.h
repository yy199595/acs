#pragma once

#include <Util/Guid.h>
#include <Util/TimeHelper.h>
#include <functional>

namespace GameKeeper
{
    class TimerBase
    {
    public:
        friend class TimerComponent;
        explicit TimerBase(long long ms);

        virtual ~TimerBase() = default;

    public:
        virtual bool Invoke() = 0;// true 表示完成 false 表示继续放进时间轮
    public:
        unsigned int GetTimerId() const { return mTimerId; }

        long long GetTriggerTime() const { return this->mTriggerTime; }

    protected:
        long long mDelayTime;
        unsigned int mTimerId;
        long long mTriggerTime;
    };
}// namespace GameKeeper