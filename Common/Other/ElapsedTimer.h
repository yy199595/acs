//
// Created by zmhy0073 on 2021/11/19.
//

#ifndef GAMEKEEPER_ELAPSEDTIMER_H
#define GAMEKEEPER_ELAPSEDTIMER_H
#include"Util/TimeHelper.h"
namespace Sentry
{
    class ElapsedTimer
    {
    public:
        ElapsedTimer() : mStartTime(Helper::Time::GetMilTimestamp()) { }
    public:
        inline long long GetMs() const;
        inline double GetSecond() const;
    private:
        const long long mStartTime;
    };

    long long ElapsedTimer::GetMs() const
    {
        long long nowTime = Helper::Time::GetMilTimestamp();
        return (nowTime - this->mStartTime);
    }

    double ElapsedTimer::GetSecond() const
    {
        long long nowTime = Helper::Time::GetMilTimestamp();
        return (nowTime - this->mStartTime) / 1000.0f;
    }

}
#endif //GAMEKEEPER_ELAPSEDTIMER_H
