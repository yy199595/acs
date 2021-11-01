#pragma once
#include<string>
namespace GameKeeper
{
	struct NetworkLatencyInfo
	{
	public:
		NetworkLatencyInfo() { memset(this, 0, sizeof(*this)); }
	public:
		int mTimeoutCount;	//超时次数
		long long mMaxLatency;	//最大延迟
		long long mMinLatency;	//最小延迟
	};
}