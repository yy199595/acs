#define _CRT_SECURE_NO_WARNINGS
#include"TimeHelper.h"
#include<chrono>
#include<ctime>
#include<sstream>
#include<regex>
#include <iomanip>
using namespace std::chrono;

namespace help
{
    long long Time::ScaleTotalTime = 0;

	bool Time::IsSameDay(time_t t1, time_t t2)
	{
		std::tm* tm1 = std::localtime(&t1);
		std::tm* tm2 = std::localtime(&t2);
		return (tm1->tm_year == tm2->tm_year && tm1->tm_mon == tm2->tm_mon && tm1->tm_mday == tm2->tm_mday);
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

	long long Time::GetTimeByString(const std::string& str)
	{
		std::tm tm = {};
		std::stringstream ss(str);
		ss >> std::get_time(&tm, TIME_FORMAT);
		std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
		return std::chrono::system_clock::to_time_t(tp);
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
        time_t t = (time_t)(time);
        struct tm *pt = localtime(&t);
        size_t size = strftime(str, sizeof(str), TIME_FORMAT, pt);
        return {str, size};
    }

	std::string Time::GetDateGMT(long long time)
	{
		time_t t = (time_t)(time);
		struct tm *gmt = localtime(&t);
		std::stringstream ss;
		ss << std::put_time(gmt, "%a, %d %b %Y %H:%M:%S GMT");
		return ss.str();
	}

	std::string Time::GetDateDT(long long timestamp)
	{
		std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(timestamp);
		std::time_t t = std::chrono::system_clock::to_time_t(tp);
		std::tm tm = *std::gmtime(&t); // 转换为 UTC 时间
		char buffer[100] = { 0 }; // 缓冲区大小
		size_t size = std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S.000+08:00", &tm); // 格式化为 ISO8601 格式
		return std::string(buffer, size);
	}

	std::string Time::GetDateISO(long long timestamp)
	{
		std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(timestamp);
		std::time_t t = std::chrono::system_clock::to_time_t(tp);
		std::tm tm = *std::gmtime(&t); // 转换为 UTC 时间
		char buffer[100] = { 0 }; // 缓冲区大小
		size_t size = std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S.000Z", &tm); // 格式化为 ISO8601 格式
		return std::string(buffer, size);
	}

    std::string Time::GetYearMonthDayString()
    {
        char str[100];
        time_t t = NowSec();
        struct tm *pt = localtime(&t);
        size_t size = strftime(str, sizeof(str), "%Y-%m-%d", pt);
        return {str, size};
    }
}
