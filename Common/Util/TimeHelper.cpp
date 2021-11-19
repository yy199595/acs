#define _CRT_SECURE_NO_WARNINGS
#include "TimeHelper.h"
#include <chrono>
#include <time.h>
using namespace std::chrono;


std::string TimeHelper::GetDateStr(long long time)
{
    time_t t = (time_t) (time == 0 ? TimeHelper::GetSecTimeStamp() : time);
    struct tm *pt = gmtime(&t);
    char str[100] = {0};
    size_t size = strftime(str, sizeof(str), "%Y%m%d%H%M%S", pt);
    return std::string(str, size);
}

long long TimeHelper::GetMilTimestamp()
{
    auto tmp = system_clock::now().time_since_epoch();
    return duration_cast<milliseconds>(tmp).count();
}

long long TimeHelper::GetSecTimeStamp()
{
    auto tmp = system_clock::now().time_since_epoch();
    return duration_cast<seconds>(tmp).count();
}

long long TimeHelper::GetMicTimeStamp()
{
    auto tmp = system_clock::now().time_since_epoch();
    return duration_cast<microseconds>(tmp).count();
}

long long TimeHelper::GetToDayZeroTime()
{
    time_t now = GetSecTimeStamp();
    struct tm *t = localtime(&now);
    t->tm_hour = 0;
    t->tm_min = 0;
    t->tm_sec = 0;
    return mktime(t) + 86400;
}

long long TimeHelper::GetTomorrowZeroTime()
{
    long long t = GetToDayZeroTime();
    return t + 24 * 60 * 60;
}

std::string TimeHelper::GetDateString(long long time)
{
    char str[100];
    time_t t = GetSecTimeStamp();
    struct tm *pt = localtime(&t);
    size_t size = strftime(str, sizeof(str), "%Y-%m-%d %H:%M:%S", pt);
    return std::string(str, size);
}

std::string TimeHelper::GetYearMonthDayString()
{
    char str[100];
    time_t t = GetSecTimeStamp();
    struct tm *pt = localtime(&t);
    size_t size = strftime(str, sizeof(str), "%Y-%m-%d", pt);
    return std::string(str, size);
}
