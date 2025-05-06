
#pragma once
#include"queue"
#include"TimerBase.h"

namespace acs
{
    class TimeWheelLayer
    {
    public:
        TimeWheelLayer(int layerId, int count, unsigned int min, unsigned int max);
    public:
		std::queue<long long> & GetTimerQueue();
        bool AddTimer(unsigned int tick, long long timerId);
		inline int GetLayerId() const { return this->mLayerId;}
		size_t GetLayerIndex() const { return this->mCurIndex;}
		inline bool JumpNextLayer() const { return this->mCurIndex >= this->mMaxCount; };
    private:
		const int mLayerId;
		const int mMaxCount;
        const unsigned int mMin;
        const unsigned int mMax;
    private:
        size_t mCurIndex;
        std::vector<std::queue<long long>> mTimerSlot;
    };
}