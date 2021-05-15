#include "TimerBase.h"
#include<iostream>
namespace SoEasy
{
	TimerBase::TimerBase(long long ms)
	{
		this->mTimerId = NumberHelper::Create();
		this->mNextInvokeTime = TimeHelper::GetMilTimestamp() + ms;
	}
}