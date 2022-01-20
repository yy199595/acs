
#pragma once

#include "TimerBase.h"

namespace Sentry
{
    class TimeWheelLayer
    {
    public:
        TimeWheelLayer(int layerId, int count, int min, int max);

        ~TimeWheelLayer();

    public:
        bool AddTimer(int tick, long long timerId);

        bool MoveIndex(std::queue<long long> &timers);

    private:
        const int mMin;
        const int mMax;
        const int mLayerId;
        const int mMaxCount;
    private:
        size_t mCurIndex;
        std::vector<std::queue<long long>> mTimerSlot;
    };
}