#pragma once
#include<functional>
#include<Util/TimeHelper.h>
#include<Util/NumberHelper.h>
namespace SoEasy
{
	class TimerBase
	{
	public:
		TimerBase(long long ms);
		virtual ~TimerBase() { }
	public:
		friend class TimerManager;
		virtual bool Invoke() = 0; //触发之后执行的操作(返回值表示是否移除, 不移除会添加到列表继续等待下次)
	public:
		const int GetTickCount() { return mTickCount; }
		const long long GetTimerId() { return mTimerId; }
	protected:
		int mTickCount;	//多少次之后执行
		long long mTimerId;
		long long mDelayTime;
	};
}