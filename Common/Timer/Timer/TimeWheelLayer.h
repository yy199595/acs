
#pragma once
#include"queue"
#include"TimerBase.h"

namespace acs
{
    class TimeWheelLayer
    {
    public:
        TimeWheelLayer(int layerId, int count, int min, int max);
    public:
        bool AddTimer(int tick, long long timerId);

        bool JumpNextLayer();
		std::queue<long long> & GetTimerQueue();
		int GetLayerId() const { return this->mLayerId;}
		size_t GetLayerIndex() const { return this->mCurIndex;}
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