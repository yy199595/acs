//
// Created by zmhy0073 on 2021/12/17.
//

#include "Guid.h"
namespace help
{
    int Guid::mIndex1 = 0;
    short Guid::mIndex2 = 0;
    long long Guid::mLastTime = 0;

    long long ThreadGuid::mLastTime = 0;
    std::atomic_int ThreadGuid::mIndex1;
    std::atomic_int16_t ThreadGuid::mIndex2;
}
namespace help
{
    long long Guid::Create()
    {
        long long nowTime = Time::NowSec();
        if (nowTime != mLastTime)
        {
            mIndex1 = 0;
            mLastTime = nowTime;
        }
        return (mLastTime << 31 | (++mIndex1));
    }

    long long Guid::Create(short type)
    {
        long long nowTime = Time::NowSec();
        if (nowTime != mLastTime)
        {
            mIndex2 = 0;
            mLastTime = nowTime;
        }
        return mLastTime << 31 | (int) type << 16 | (++mIndex2);
    }
}

namespace help
{
    long long ThreadGuid::Create()
    {
        long long nowTime = Time::NowSec();
        if (nowTime != mLastTime)
        {
            mIndex1 = 0;
            mLastTime = nowTime;
        }
        return mLastTime << 31 | (++mIndex1);
    }

    long long ThreadGuid::Create(short type)
    {
        long long nowTime = Time::NowSec();
        if (nowTime != mLastTime)
        {
            mIndex2 = 0;
            mLastTime = nowTime;
        }
        return mLastTime << 32 | (int) type << 16 | (++mIndex2);
    }
}