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
        long long GetTimerId() const { return mTimerId; }
        long long GetStartTime() const { return this->mStartTime;}
        long long GetTargetTime() const { return this->mTargetTime; }
    private:
        long long mTimerId;
        long long mStartTime;
        long long mTargetTime;
    };
}// namespace GameKeeper