#pragma once
#include<atomic>
#include<string>

namespace help
{
    class ID
    {
    public:
        static long long Create();
        static long long Create(int id);
		static long long Create(int id, int count);
    private:
        static int mIndex1;
        static short mIndex2;
        static long long mLastTime;
    };
}// namespace Guid