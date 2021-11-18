
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
        bool AddTimer(int tick, unsigned int timer);

        bool MoveIndex(std::queue<unsigned int> &timers);

    private:
        const int mLayerId;
        const int mMaxCount;
        const int mEnd;
        const int mStart;

    private:
        size_t mCurIndex;
        std::vector<std::queue<unsigned int>> mTimerSlot;
    };
}