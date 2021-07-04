#pragma once

#include"TimerBase.h"
namespace SoEasy
{
	class DelayTimer : public TimerBase
	{
	public:
		DelayTimer(long long ms, std::function<void(void)> & func)
			: TimerBase(ms), mBindAction(func) { }
	public:
		bool Invoke() final { this->mBindAction(); return true; }
	private:
		std::function<void(void)> mBindAction;
	};
}