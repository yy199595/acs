#pragma once
#include<atomic>
#include"Util/Tools/TimeHelper.h"
namespace help
{
    class Guid
    {
    public:
        static long long Create();
        long long Create(short type);
    private:
        static int mIndex1;
        static short mIndex2;
        static long long mLastTime;
    };

    class ThreadGuid
    {
    public:
        static long long Create();
        long long Create(short type);
    private:
        static long long mLastTime;
        static std::atomic_int mIndex1;
        static std::atomic_int16_t mIndex2;
    };
}// namespace Guid