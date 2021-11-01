#pragma once

#include "TimerBase.h"
#include<Method/MethodProxy.h>
namespace GameKeeper
{
	class DelayTimer : public TimerBase
	{
	public:
		DelayTimer(long long ms, StaticMethod * func)
			: TimerBase(ms), mFunc(func) {}

	public:
		bool Invoke() final
		{
			this->mFunc->run();
			delete mFunc;
			this->mFunc = nullptr;
			return true;
		}
	private:
		StaticMethod * mFunc;
	};
}// namespace GameKeeper