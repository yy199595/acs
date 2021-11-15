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
        ~DelayTimer() final { delete this->mFunc;}
	public:
		bool Invoke() final
		{
			this->mFunc->run();
			return true;
		}
	private:
		StaticMethod * mFunc;
	};
}// namespace GameKeeper