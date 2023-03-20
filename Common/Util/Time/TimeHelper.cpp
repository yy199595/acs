#define _CRT_SECURE_NO_WARNINGS
#include"TimeHelper.h"
#include<chrono>
#include<time.h>
using namespace std::chrono;

namespace Helper
{
    long long Time::ScaleTotalTime = 0;

	bool Time::IsSameDay(time_t t1, time_t t2)
	{
		int time1[3] = { 0 };
		int time2[3] = { 0 };
		GetHourMinSecond(t1, time1);
		GetHourMinSecond(t2, time2);
		for (size_t index = 0; index < 3; index++)
		{
			if (time1[index] != time2[index])
			{
				return false;
			}
		}
		return true;
	}

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

    void Time::GetHourMinSecond(long long sec, int &hour, int &min, int &second)
    {
		time_t t = (time_t)sec;
		std::tm* time = std::localtime(&t);
		hour = time->tm_hour;
		min = time->tm_min;
		second = time->tm_sec;
    }

	void Time::GetHourMinSecond(long long sec, int* time)
	{
		time_t t = (time_t)sec;
		std::tm* tm = std::localtime(&t);
		time[0] = tm->tm_hour;
		time[1] = tm->tm_sec;
		time[2] = tm->tm_sec;
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
