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
                             ? Time::NowSecTime() : time);
        struct tm *pt = gmtime(&t);
        size_t size = strftime(str, sizeof(str), "%Y%m%d%H%M%S", pt);
        return std::string(str, size);
    }

    long long Time::NowMilTime()
    {
        auto tmp = system_clock::now().time_since_epoch();
        return duration_cast<milliseconds>(tmp).count() + Time::ScaleTotalTime * 1000;
    }

    long long Time::NowSecTime()
    {
        auto tmp = system_clock::now().time_since_epoch();
        return duration_cast<seconds>(tmp).count() + Time::ScaleTotalTime;
    }

    long long Time::NowMicTime()
    {
        auto tmp = system_clock::now().time_since_epoch();
        return duration_cast<microseconds>(tmp).count();
    }

    long long Time::GetNewTime(int day, int hour, int minute, int second)
    {
        time_t now = NowSecTime();
        struct tm *t = localtime(&now);
        t->tm_hour = hour;
        t->tm_min = minute;
        t->tm_sec = second;
        return mktime(t) + Time::DaySecond * day;
    }

    void Time::GetHourMinSecond(const long long sec, int &hour, int &min, int &second)
    {
        hour = sec / Time::HourSecond;
        min = (sec - hour * Time::HourSecond) / Time::MinSecond;
        second = sec % 60;
    }

    std::string Time::GetDateString(long long time)
    {
        char str[100];
        time_t t = NowSecTime();
        struct tm *pt = localtime(&t);
        size_t size = strftime(str, sizeof(str), "%Y-%m-%d %H:%M:%S", pt);
        return {str, size};
    }

    std::string Time::GetYearMonthDayString()
    {
        char str[100];
        time_t t = NowSecTime();
        struct tm *pt = localtime(&t);
        size_t size = strftime(str, sizeof(str), "%Y-%m-%d", pt);
        return {str, size};
    }
}
