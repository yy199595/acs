//
// Created by zmhy0073 on 2021/11/19.
//

#ifndef GAMEKEEPER_ELAPSEDTIMER_H
#define GAMEKEEPER_ELAPSEDTIMER_H
#include"Util/TimeHelper.h"
namespace GameKeeper
{
    class ElapsedTimer
    {
    public:
        ElapsedTimer() : mStartTime(TimeHelper::GetMilTimestamp()) { }
    public:
        inline double GetMs() const;
        inline double GetSecond() const;
    private:
        const long long mStartTime;
    };

    double ElapsedTimer::GetMs() const
    {
        long long nowTime = TimeHelper::GetMilTimestamp();
        return (double)(nowTime - this->mStartTime);
    }

    double ElapsedTimer::GetSecond() const
    {
        long long nowTime = TimeHelper::GetMilTimestamp();
        return (nowTime - this->mStartTime) / 1000.0f;
    }

}
#endif //GAMEKEEPER_ELAPSEDTIMER_H
