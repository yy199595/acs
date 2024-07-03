//
// Created by zmhy0073 on 2021/11/19.
//

#ifndef APP_ELAPSEDTIMER_H
#define APP_ELAPSEDTIMER_H
#include"Util/Time/TimeHelper.h"
namespace timer
{
    class ElapsedTimer
    {
    public:
        ElapsedTimer() : mStartTime(help::Time::NowMil()) { }
    public:
        inline long long GetMs() const;
        inline double GetSecond() const;
    private:
        const long long mStartTime;
    };

    long long ElapsedTimer::GetMs() const
    {
        long long nowTime = help::Time::NowMil();
        return (nowTime - this->mStartTime);
    }

    double ElapsedTimer::GetSecond() const
    {
        long long nowTime = help::Time::NowMil();
        return (nowTime - this->mStartTime) / 1000.0f;
    }

}
#endif //APP_ELAPSEDTIMER_H
