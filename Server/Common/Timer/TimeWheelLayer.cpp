#include "TimeWheelLayer.h"

namespace Sentry
{
    TimeWheelLayer::TimeWheelLayer(int layerId, int count, int start, int end)
        : mLayerId(layerId), mMaxCount(count), mStart(start), mEnd(end)
    {
        this->mCurIndex = 0;
        for (int index = 0; index < count; index++)
        {
            std::queue<shared_ptr<TimerBase>> timers;
            this->mTimerSlot.push_back(timers);
        }
    }
    bool TimeWheelLayer::AddTimer(int tick, shared_ptr<TimerBase> timer)
    {
        if (timer == nullptr)
        {
            return false;
        }

        if (tick >= this->mStart && tick < this->mEnd)
        {
            int index = this->mStart == 0 ? tick
                    : (tick - this->mStart) / this->mStart;

            if(index + this->mCurIndex < this->mMaxCount)
            {
                index+= this->mCurIndex;
                this->mTimerSlot[index].push(timer);
            }
            else
            {
                index = index + this->mCurIndex - this->mMaxCount;
                 this->mTimerSlot[index].push(timer);
            }
            return true;
        }
        return false;
    }
    bool TimeWheelLayer::MoveIndex(std::queue<shared_ptr<TimerBase>> &timers)
    {
        timers = this->mTimerSlot[this->mCurIndex];
        if ((++this->mCurIndex) >= this->mMaxCount)
        {
            this->mCurIndex = 0;
            return true;
        }
        return false;
    }
}