#include"TimeWheelLayer.h"
namespace acs
{
    TimeWheelLayer::TimeWheelLayer(int layerId, int count, unsigned int min,  unsigned int max)
        : mMin(min), mMax(max), mLayerId(layerId), mMaxCount(count)
    {
        this->mCurIndex = 0;
        for (int index = 0; index < count; index++)
        {
            std::queue<long long> timers;
            this->mTimerSlot.push_back(timers);
        }
    }

    bool TimeWheelLayer::AddTimer(unsigned int tick, long long timerId)
	{
		if (tick >= this->mMin && tick < this->mMax)
		{
			unsigned int index = this->mMin == 0 ? tick : (tick - this->mMin) / this->mMin;

			if (index + this->mCurIndex < this->mMaxCount)
			{
				index += this->mCurIndex;
				this->mTimerSlot[index].push(timerId);
			}
			else
			{
				index = index + this->mCurIndex - this->mMaxCount;
				this->mTimerSlot[index].push(timerId);
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
}// namespace Sentry