#pragma once
#include<time.h>
#include<string>

namespace Helper
{
    namespace Time
    {
		constexpr int MinSecond = 60;
		constexpr int HourSecond = 3600;
		constexpr int DaySecond = 86400;

        extern long long ScaleTotalTime;

        extern void SetScaleTotalTime(int second);

        extern std::string GetDateStr(long long time = 0);

        // 获取毫秒时间戳
        extern long long NowMilTime();

        // 获取秒级时间戳
        extern long long NowSecTime();

        // 获取微妙级时间戳
        extern long long NowMicTime();

        // 获取时间字符串 精确到秒
        extern std::string GetDateString(long long t = time(nullptr));

        // 获取时间字符串精确到天
        extern std::string GetYearMonthDayString();

        //获取今天零点时间戳
        extern long long GetNewTime(int day = 0, int hour = 0, int minute = 0, int second = 0);

        // 获取十分秒
        extern void GetHourMinSecond(long long sec, int &hour, int &min, int &second);
    }
// namespace Helper::Time
}