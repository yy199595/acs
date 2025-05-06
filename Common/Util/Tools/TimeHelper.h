#pragma once
#include<ctime>
#include<string>

#define TIME_FORMAT "%Y-%m-%d %H:%M:%S"
namespace help
{
    namespace Time
    {
		struct Date
		{
			int Year = 0;
			int Month = 0;
			int Day = 0;
			int Hour = 0;
			int Minute = 0;
			int Second = 0;
			int Milliseconds = 0;
			int Week = 0;
		};


		constexpr int MinSecond = 60;
		constexpr int HourSecond = 60 * 60;
		constexpr int DaySecond = 60 * 60 * 24;

        extern long long ScaleTotalTime;

		extern bool IsSameDay(long long t1, long long t2);

        extern void SetScaleTotalTime(int second);

        extern std::string GetDateStr(long long time = 0);

		extern void GetTimeDate(Time::Date & date, long long t = 0);

        // 获取毫秒时间戳
        extern long long NowMil();

        // 获取秒级时间戳
        extern long long NowSec();

        // 获取微妙级时间戳
        extern long long NowMic();

		//获取gmt时间
		extern std::string GetDateGMT(long long t = 0);

		extern std::string GetDateISO(long long t = 0);

		extern std::string GetDateDT(long long t = 0);

		// 获取时间字符串 精确到秒
        extern std::string GetDateString(long long t = 0);


        // 获取时间字符串精确到天
        extern std::string GetYearMonthDayString(long long t = 0);

		// 根据字符串获取时间戳
		extern long long GetTimeByString(const char * str);

        //获取今天零点时间戳
        extern long long GetNewTime(int day = 0, int hour = 0, int minute = 0, int second = 0);

        // 获取十分秒
		extern void GetHourMinSecond(long long sec, int * time);

		extern void GetHourMinSecond(long long sec, int &hour, int &min, int &second);

		extern Time::Date CalcHourMinSecond(long long sec);
    }

	namespace HighTime
	{
		extern long long NowMil();
		extern long long NowSec();
	}
// namespace Helper::Time
}