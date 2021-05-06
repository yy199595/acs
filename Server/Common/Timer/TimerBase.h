#pragma once
#include<functional>
#include<Util/TimeHelper.h>
#include<Util/NumberHelper.h>
namespace SoEasy
{
	class TimerBase
	{
	public:
		TimerBase(long long ms) { mNextInvokeTime = TimeHelper::GetMilTimestamp() + ms; }
		virtual ~TimerBase() { }
	public:
		virtual bool Invoke() = 0; //触发之后执行的操作(返回值表示是否移除, 不移除会添加到列表继续等待下次)
		virtual bool IsTrigger(long long nowTime) { return nowTime >= this->mNextInvokeTime; }
	public:
		const long long GetTimerId() { return mTimerId; }
		const long long GetNextInvokeTime() { return mNextInvokeTime; }
		void MoveNextInvokeTime(long long ms) { this->mNextInvokeTime += ms; }
	private:
		long long mTimerId;
		long long mNextInvokeTime;
	};
}