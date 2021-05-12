#define  _CRT_SECURE_NO_WARNINGS
#include"TimeHelper.h"
#include<chrono>
#include<time.h>
using namespace std::chrono;



std::string TimeHelper::GetDateStr(long long time)
{
	time_t t = (time_t)(time == 0 ? TimeHelper::GetSecTimeStamp() : time);
	struct tm *pt = gmtime(&t);
	char str[100] = { 0 };
	size_t size = strftime(str, sizeof(str), "%Y%m%d%H%M%S", pt);
	return std::string(str, size);
}

long long TimeHelper::GetMilTimestamp()
{
	auto tmp = system_clock::now().time_since_epoch();
	auto timeNow = duration_cast<milliseconds>(tmp);
	return timeNow.count();
}

long long TimeHelper::GetSecTimeStamp()
{
	auto tmp = system_clock::now().time_since_epoch();
	auto timeNow = duration_cast<seconds>(tmp);
	return timeNow.count();
}

long long TimeHelper::GetMicTimeStamp()
{
	auto tmp = system_clock::now().time_since_epoch();
	auto timeNow = duration_cast<microseconds>(tmp);
	return timeNow.count();
}

std::string TimeHelper::GetDateString(long long time)
{
	time_t t = time + 28800;
	struct tm *pt = gmtime(&t);
	char str[100];
	size_t size = strftime(str, sizeof(str), "%Y-%m-%d %H:%M:%S", pt);
	return std::string(str, size);
}

std::string TimeHelper::GetYearMonthDayString()
{
	time_t t = time(NULL) + 28800;
	struct tm *pt = gmtime(&t);
	char str[100];
	size_t size = strftime(str, sizeof(str), "%Y%m%d", pt);
	return std::string(str, size);
}
