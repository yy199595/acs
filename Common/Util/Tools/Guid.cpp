//
// Created by zmhy0073 on 2021/12/17.
//

#include "Guid.h"
#include "Util/Crypt/md5.h"
#include"Util/Tools/TimeHelper.h"
namespace help
{
    int ID::mIndex1 = 0;
    short ID::mIndex2 = 0;
    long long ID::mLastTime = 0;

    long long ThreadGuid::mLastTime = 0;
    std::atomic_int ThreadGuid::mIndex1;
    std::atomic_int16_t ThreadGuid::mIndex2;
}
namespace help
{
    std::string ID::UUid()
    {
        std::string str = std::to_string(ID::Create());
        return help::md5::GetMd5(str);
    }

    long long ID::Create()
    {
        long long nowTime = Time::NowSec();
        if (nowTime != mLastTime)
        {
            mIndex1 = 0;
            mLastTime = nowTime;
        }
        return (mLastTime << 31 | (++mIndex1));
    }

    long long ID::Create(short type)
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