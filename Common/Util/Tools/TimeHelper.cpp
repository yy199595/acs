﻿
#include"TimeHelper.h"
#include<chrono>
#include<ctime>
#include<sstream>
#include <iomanip>

using namespace std::chrono;

namespace help
{
    long long Time::ScaleTotalTime = 0;

	void Time::GetTimeDate(Time::Date & timeDate, long long t)
	{
		time_t t1 = t == 0 ? NowSec() : t;
		std::tm * tm1 = std::localtime(&t1);
		{
			timeDate.Day = tm1->tm_mday;
			timeDate.Hour = tm1->tm_hour;
			timeDate.Minute = tm1->tm_min;
			timeDate.Second = tm1->tm_sec;
			timeDate.Month = tm1->tm_mon + 1;
			timeDate.Week = tm1->tm_wday + 1;
			timeDate.Year = tm1->tm_year + 1900;
		}
	}

	bool Time::IsSameDay(long long t1, long long t2)
	{
		Time::Date timeData1;
		Time::Date timeData2;
		GetTimeDate(timeData1, t1);
		GetTimeDate(timeData2, t2);
		return timeData1.Year == timeData2.Year && timeData1.Month == timeData2.Month && timeData1.Day == timeData2.Day;
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
							 ? Time::NowSec() : time);
        struct tm *pt = gmtime(&t);
        size_t size = strftime(str, sizeof(str), "%Y%m%d%H%M%S", pt);
        return std::string(str, size);
    }

	long long Time::GetTimeByString(const char * timeStr)
	{
		std::tm tm = {};
		std::stringstream ss(timeStr);
		ss >> std::get_time(&tm, TIME_FORMAT);
		std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
		return (long long)std::chrono::system_clock::to_time_t(tp);
	}

    long long Time::NowMil()
    {
        auto tmp = system_clock::now().time_since_epoch();
        return duration_cast<milliseconds>(tmp).count() + Time::ScaleTotalTime * 1000;
    }

    long long Time::NowSec()
    {
        auto tmp = system_clock::now().time_since_epoch();
        return duration_cast<seconds>(tmp).count() + Time::ScaleTotalTime;
    }

    long long Time::NowMic()
    {
        auto tmp = system_clock::now().time_since_epoch();
        return duration_cast<microseconds>(tmp).count();
    }

    long long Time::GetNewTime(int day, int hour, int minute, int second)
    {
        time_t now = NowSec();
        struct tm *t = localtime(&now);
		{
			t->tm_hour = hour;
			t->tm_min = minute;
			t->tm_sec = second;
		}
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
        time_t t = time == 0 ? NowSec() : time;
        struct tm *pt = localtime(&t);
		if(pt == nullptr)
		{
			return "";
		}
        size_t size = strftime(str, sizeof(str), TIME_FORMAT, pt);
        return {str, size};
    }

	std::string Time::GetDateGMT(long long time)
	{
		time_t t = time == 0 ? NowSec() : time;

		struct tm *gmt = localtime(&t);
		std::stringstream ss;
		ss << std::put_time(gmt, "%a, %d %b %Y %H:%M:%S GMT");
		return ss.str();
	}

	std::string Time::GetDateDT(long long timestamp)
	{
		if(timestamp == 0)
		{
			timestamp = NowSec();
		}
		std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(timestamp);
		std::time_t t = std::chrono::system_clock::to_time_t(tp);
		std::tm tm = *std::gmtime(&t); // 转换为 UTC 时间
		char buffer[100] = { 0 }; // 缓冲区大小
		size_t size = std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S.000+08:00", &tm); // 格式化为 ISO8601 格式
		return std::string(buffer, size);
	}

	std::string Time::GetDateISO(long long timestamp)
	{
		if(timestamp == 0)
		{
			timestamp = NowSec();
		}
		std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(timestamp);
		std::time_t t = std::chrono::system_clock::to_time_t(tp);
		std::tm tm = *std::gmtime(&t); // 转换为 UTC 时间
		char buffer[100] = { 0 }; // 缓冲区大小
		size_t size = std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S.000Z", &tm); // 格式化为 ISO8601 格式
		return std::string(buffer, size);
	}

    std::string Time::GetYearMonthDayString(long long t)
    {
        char str[100] = { 0 };
        if(t == 0)
		{
			t = NowSec();
		}
		time_t t1 = t;
        struct tm *pt = localtime(&t1);
        size_t size = strftime(str, sizeof(str), "%Y-%m-%d", pt);
        return {str, size};
    }

	Time::Date Time::CalcHourMinSecond(long long totalMilliseconds)
	{
		Time::Date timeData;
		constexpr int millisecondsInSecond = 1000;
		constexpr int secondsInMinute = 60;
		constexpr int secondsInHour = 60 * secondsInMinute;
		constexpr int secondsInDay = 24 * secondsInHour;
		constexpr int daysInMonth = 30;
		constexpr int daysInYear = 365;

		timeData.Milliseconds = totalMilliseconds % millisecondsInSecond;
		totalMilliseconds /= millisecondsInSecond;

		// 计算年数
		timeData.Year = totalMilliseconds / (secondsInDay * daysInYear);
		totalMilliseconds %= (secondsInDay * daysInYear);

		// 计算月数
		timeData.Month = totalMilliseconds / (secondsInDay * daysInMonth);
		totalMilliseconds %= (secondsInDay * daysInMonth);

		// 计算天数
		timeData.Day = totalMilliseconds / secondsInDay;
		totalMilliseconds %= secondsInDay;

		// 计算小时数
		timeData.Hour = totalMilliseconds / secondsInHour;
		totalMilliseconds %= secondsInHour;

		// 计算分钟数
		timeData.Minute = totalMilliseconds / secondsInMinute;

		// 剩余的秒数
		timeData.Second = totalMilliseconds % secondsInMinute;
		return timeData;
	}

	long long HighTime::NowSec()
	{
		auto tmp = high_resolution_clock ::now().time_since_epoch();
		return duration_cast<std::chrono::seconds>(tmp).count() + Time::ScaleTotalTime * 1000;
	}

	long long HighTime::NowMil()
	{
		auto tmp = high_resolution_clock::now().time_since_epoch();
		return duration_cast<milliseconds>(tmp).count() + Time::ScaleTotalTime * 1000;
	}
}
