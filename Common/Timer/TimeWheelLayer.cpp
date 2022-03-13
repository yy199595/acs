#include"TimeWheelLayer.h"
#include"Object/App.h"
#include"Define/CommonLogDef.h"
namespace Sentry
{
    TimeWheelLayer::TimeWheelLayer(int layerId, int count, int min, int max)
        : mLayerId(layerId), mMaxCount(count), mMin(min), mMax(max)
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
            int index = this->mMin == 0 ? tick
                                          : (tick - this->mMin) / this->mMin;

            if (index + this->mCurIndex < this->mMaxCount)
            {
                index += this->mCurIndex;
                this->mTimerSlot[index].push(timerId);
            } else
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
		return this->mTimerSlot[this->mCurIndex];
	}

    bool TimeWheelLayer::MoveIndex()
    {
        if ((++this->mCurIndex) >= this->mMaxCount)
        {
            this->mCurIndex = 0;
            return true;
        }
        return false;
    }
}// namespace Sentry