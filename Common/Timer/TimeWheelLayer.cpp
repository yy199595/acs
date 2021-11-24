#include "TimeWheelLayer.h"

namespace GameKeeper
{
    TimeWheelLayer::TimeWheelLayer(int layerId, int count, int start, int end)
        : mLayerId(layerId), mMaxCount(count), mStart(start), mEnd(end)
    {
        this->mCurIndex = 0;
        for (int index = 0; index < count; index++)
        {
            std::queue<unsigned int> timers;
            this->mTimerSlot.push_back(timers);
        }
    }

    bool TimeWheelLayer::AddTimer(int tick, unsigned int timer)
    {
        if (timer == 0)
        {
            return false;
        }

        if (tick >= this->mStart && tick < this->mEnd)
        {
            int index = this->mStart == 0 ? tick
                                          : (tick - this->mStart) / this->mStart;

            if (index + this->mCurIndex < this->mMaxCount)
            {
                index += this->mCurIndex;
                this->mTimerSlot[index].push(timer);
            } else
            {
                index = index + this->mCurIndex - this->mMaxCount;
                this->mTimerSlot[index].push(timer);
            }
            return true;
        }
        return false;
    }

    bool TimeWheelLayer::MoveIndex(std::queue<unsigned int> &timers)
    {
        std::swap(timers, this->mTimerSlot[this->mCurIndex]);
        if ((++this->mCurIndex) >= this->mMaxCount)
        {
            this->mCurIndex = 0;
            return true;
        }
        return false;
    }
}// namespace GameKeeper