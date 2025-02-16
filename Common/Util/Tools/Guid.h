#pragma once
#include<atomic>
#include<string>

namespace help
{
    class ID
    {
    public:
        static long long Create();
        static long long Create(short type);
		static int New(long long startTime, int id);
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