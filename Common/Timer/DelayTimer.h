#pragma once

#include "TimerBase.h"
#include<Method/MethodProxy.h>
namespace Sentry
{
	class DelayTimer : public TimerBase
	{
	public:
		DelayTimer(long long ms, StaticMethod * func)
			: TimerBase(ms), mFunc(func) {}
        ~DelayTimer() final { delete this->mFunc;}
	public:
		void Invoke() final
		{
			this->mFunc->run();
		}
	private:
		StaticMethod * mFunc;
	};
}// namespace Sentry