
#pragma once
#include"queue"
#include"TimerBase.h"

namespace joke
{
    class TimeWheelLayer
    {
    public:
        TimeWheelLayer(int layerId, int count, int min, int max);
    public:
        bool AddTimer(int tick, long long timerId);

        bool JumpNextLayer();
		std::queue<long long> & GetTimerQueue();
		const int GetLayerId() { return this->mLayerId;}
		const size_t GetLayerIndex() { return this->mCurIndex;}
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