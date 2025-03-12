//
// Created by zmhy0073 on 2021/12/17.
//

#include "Guid.h"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <random>
#include "Util/Tools/TimeHelper.h"

namespace help
{
	int ID::mIndex1 = 0;
	short ID::mIndex2 = 0;
	long long ID::mLastTime = 0;
}
namespace help
{

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

	long long ID::Create(int id)
	{
		long long nowTime = Time::NowSec();
		if (nowTime != mLastTime)
		{
			mIndex2 = 0;
			mLastTime = nowTime;
		}
		return mLastTime << 31 | (int)id << 16 | (++mIndex2);
	}

	long long ID::Create(int id, int count)
	{
		int num = 1;
		for (int index = 0; index < count; index++)
		{
			num *= 10;
		}
		long long guid = ID::Create(id);
		return guid % num;
	}
}
