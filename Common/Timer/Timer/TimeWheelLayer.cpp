#include"TimeWheelLayer.h"
namespace acs
{
    TimeWheelLayer::TimeWheelLayer(int layerId, int count, int min, int max)
        : mMin(min), mMax(max), mLayerId(layerId), mMaxCount(count)
    {
        this->mCurIndex = 0;
        for (int index = 0; index < count; index++)
        {
            std::queue<long long> timers;
            this->mTimerSlot.push_back(timers);
        }
    }

    bool TimeWheelLayer::AddTimer(int tick, long long timerId)
	{
		if (tick >= this->mMin && tick < this->mMax)
		{
			int index = this->mMin == 0 ? tick : (tick - this->mMin) / this->mMin;

			if (index + this->mCurIndex < this->mMaxCount)
			{
				index += this->mCurIndex;
				this->mTimerSlot[index].push(timerId);
//				printf("[layer = %d] [tick = %d] [curIndex = %d] [slot = %d]\n",
//					this->mLayerId, tick, this->mCurIndex, index);
			}
			else
			{
				index = index + this->mCurIndex - this->mMaxCount;
				this->mTimerSlot[index].push(timerId);
//				printf("[layer = %d] [tick = %d] [curIndex = %d] [slot = %d]\n",
//					this->mLayerId, tick, this->mCurIndex, index);
			}
			return true;
		}
		return false;
	}

	std::queue<long long> & TimeWheelLayer::GetTimerQueue()
	{
		if(this->mCurIndex >= this->mMaxCount)
		{
			this->mCurIndex = 0;
		}
		return this->mTimerSlot[this->mCurIndex++];
	}

    bool TimeWheelLayer::JumpNextLayer()
    {
        return this->mCurIndex >= this->mMaxCount;
    }
}// namespace Sentry