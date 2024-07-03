#pragma once
#include<ctime>
#include<string>

#define TIME_FORMAT "%Y-%m-%d %H:%M:%S"
namespace help
{
    namespace Time
    {
		constexpr int MinSecond = 60;
		constexpr int HourSecond = 3600;
		constexpr int DaySecond = 86400;

        extern long long ScaleTotalTime;

		extern bool IsSameDay(time_t t1, time_t t2);

        extern void SetScaleTotalTime(int second);

        extern std::string GetDateStr(long long time = 0);

        // 获取毫秒时间戳
        extern long long NowMil();

        // 获取秒级时间戳
        extern long long NowSec();

        // 获取微妙级时间戳
        extern long long NowMic();

		//获取gmt时间
		extern std::string GetDateGMT(long long t = time(nullptr));

		extern std::string GetDateISO(long long t = time(nullptr));

		extern std::string GetDateDT(long long t = time(nullptr));

		// 获取时间字符串 精确到秒
        extern std::string GetDateString(long long t = time(nullptr));


        // 获取时间字符串精确到天
        extern std::string GetYearMonthDayString();

		// 根据字符串获取时间戳
		extern long long GetTimeByString(const std::string & str);

        //获取今天零点时间戳
        extern long long GetNewTime(int day = 0, int hour = 0, int minute = 0, int second = 0);

        // 获取十分秒
		extern void GetHourMinSecond(long long sec, int * time);
		extern void GetHourMinSecond(long long sec, int &hour, int &min, int &second);
    }
// namespace Helper::Time
}