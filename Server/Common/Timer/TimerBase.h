#pragma once
#include<functional>
#include<Util/TimeHelper.h>
#include<Util/NumberHelper.h>
namespace Sentry
{
	class TimerBase
	{
	public:
		TimerBase(long long ms);
		virtual ~TimerBase() { }
	public:
		virtual bool Invoke() = 0; //触发之后执行的操作(返回值表示是否移除, 不移除会添加到列表继续等待下次)
	public:
		const long long GetTimerId() { return mTimerId; }
		const long long GetTriggerTime() { return this->mTriggerTime;}
	protected:
		long long mTimerId;
		long long mDelayTime;
		long long mTriggerTime;
	};
}