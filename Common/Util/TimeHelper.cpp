#define _CRT_SECURE_NO_WARNINGS
#include"TimeHelper.h"
#include<chrono>
#include<time.h>
using namespace std::chrono;

namespace Helper
{
    long long Time::ScaleTotalTime = 0;

    void Time::SetScaleTotalTime(int second)
    {
        if (second == 0)
        {
            Time::ScaleTotalTime = 0;
            return;
        }
        Time::ScaleTotalTime += second;
    }
    std::string Time::GetDateStr(long long time)
    {
        char str[100] = {0};
        time_t t = (time_t) (time == 0
                ? Time::GetSecTimeStamp() : time);
        struct tm *pt = gmtime(&t);
        size_t size = strftime(str, sizeof(str), "%Y%m%d%H%M%S", pt);
        return std::string(str, size);
    }

    long long Time::GetMilTimestamp()
    {
        auto tmp = system_clock::now().time_since_epoch();
        return duration_cast<milliseconds>(tmp).count() + Time::ScaleTotalTime * 1000;
    }

    long long Time::GetSecTimeStamp()
    {
        auto tmp = system_clock::now().time_since_epoch();
        return duration_cast<seconds>(tmp).count() + Time::ScaleTotalTime;
    }

    long long Time::GetMicTimeStamp()
    {
        auto tmp = system_clock::now().time_since_epoch();
        return duration_cast<microseconds>(tmp).count();
    }

    long long Time::GetToDayZeroTime()
    {
        time_t now = GetSecTimeStamp();
        struct tm *t = localtime(&now);
        t->tm_hour = 0;
        t->tm_min = 0;
        t->tm_sec = 0;
        return mktime(t) + _DaySecond;
    }

    long long Time::GetTomorrowZeroTime()
    {
        long long t = GetToDayZeroTime();
        return t + _DaySecond;
    }

    void Time::GetHourMinSecond(const long long sec, int &hour, int &min, int &second)
    {
        hour = sec / _HourSecond;
        min = (sec - hour * _HourSecond) / _MinSecond;
        second = sec % 60;
    }

    std::string Time::GetDateString(long long time)
    {
        char str[100];
        time_t t = GetSecTimeStamp();
        struct tm *pt = localtime(&t);
        size_t size = strftime(str, sizeof(str), "%Y-%m-%d %H:%M:%S", pt);
        return {str, size};
    }

    std::string Time::GetYearMonthDayString()
    {
        char str[100];
        time_t t = GetSecTimeStamp();
        struct tm *pt = localtime(&t);
        size_t size = strftime(str, sizeof(str), "%Y-%m-%d", pt);
        return {str, size};
    }
}
