#include "TimerBase.h"
#include<iostream>
namespace SoEasy
{
	TimerBase::TimerBase(long long ms)
	{
		this->mDelayTime = ms;
		this->mTickCount = 0;
		this->mTimerId = NumberHelper::Create();		
	}
}