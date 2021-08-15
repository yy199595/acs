
#pragma once

#include "TimerBase.h"

namespace Sentry
{
    class TimeWheelLayer
    {
    public:
        TimeWheelLayer(int layerId, int count, int start, int end);

        ~TimeWheelLayer();

    public:
        bool AddTimer(int tick, shared_ptr<TimerBase> timer);

        bool MoveIndex(std::queue<shared_ptr<TimerBase>> &timers);

    private:
        const int mLayerId;
        const int mMaxCount;
        const int mEnd;
        const int mStart;

    private:
        size_t mCurIndex;
        std::vector<std::queue<shared_ptr<TimerBase>>> mTimerSlot;
    };
}