
#pragma once

#include "TimerBase.h"

namespace GameKeeper
{
    class TimeWheelLayer
    {
    public:
        TimeWheelLayer(int layerId, int count, int start, int end);

        ~TimeWheelLayer();

    public:
        bool AddTimer(int tick, TimerBase * timer);

        bool MoveIndex(std::queue<TimerBase *> &timers);

    private:
        const int mLayerId;
        const int mMaxCount;
        const int mEnd;
        const int mStart;

    private:
        size_t mCurIndex;
        std::vector<std::queue<TimerBase *>> mTimerSlot;
    };
}