#pragma once
#include <Define/CommonTypeDef.h>
#include <string>
namespace TimeHelper
{
    extern std::string GetDateStr(long long time = 0);

    // 获取毫秒时间戳
    extern long long GetMilTimestamp();

    // 获取秒级时间戳
    extern long long GetSecTimeStamp();

    // 获取微妙级时间戳
    //extern long long GetMicTimeStamp();

    // 获取时间字符串 精确到秒
    extern std::string GetDateString(long long t = time(NULL));

    // 获取时间字符串精确到天
    extern std::string GetYearMonthDayString();

    //获取今天零点时间戳
    extern long long GetToDayZeroTime();

    //获取明天零点时间戳
    extern long long GetTomorrowZeroTime();
}// namespace TimeHelper