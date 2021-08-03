#include "NumberHelper.h"
#include <Util/TimeHelper.h>
namespace NumberHelper
{
    long long Combination(const int num1, const int num2)
    {
        return (long long) num1 << 32 | num2;
    }

    long long Create(unsigned short serverId)
    {
        static short index = 0;
        long long nowTime = TimeHelper::GetSecTimeStamp();
        static long long lastTime = TimeHelper::GetSecTimeStamp();
        if (lastTime != nowTime)
        {
            index = 0;
            lastTime = nowTime;
        }
        return lastTime << 32 | (int) serverId << 16 | (++index);
    }
}// namespace NumberHelper