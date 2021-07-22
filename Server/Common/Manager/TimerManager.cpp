#include "TimerManager.h"
#include <Timer/DelayTimer.h>
#include <Util/TimeHelper.h>
namespace Sentry
{
	TimerManager::TimerManager()
	{
		this->mTickCount = 0;
		this->mTimerCount = 0;
		this->mMaxTimeCount = MaxMinute * 60 * (1000 / TimerPrecision);
	}

	bool TimerManager::AddTimer(shared_ptr<TimerBase> timer)
	{
		if (timer == nullptr)
		{
			return false;
		}
		if (timer->mDelayTime <= 0)
		{
			timer->Invoke();
			return true;
		}
		long long id = timer->GetTimerId();
		auto iter = this->mTimerMap.find(id);
		if (iter == this->mTimerMap.end())
		{
			this->AddTimerToWheel(timer);
			this->mTimerMap.emplace(id, timer);
			return true;
		}
		return false;
	}

	bool TimerManager::AddTimer(long long ms, std::function<void(void)> func)
	{
		if (ms == 0)
		{
			func();
			return true;
		}
		return this->AddTimer(make_shared<DelayTimer>(ms, func));
	}

	bool TimerManager::RemoveTimer(long long id)
	{
		auto iter = this->mTimerMap.find(id);
		if (iter != this->mTimerMap.end())
		{
			shared_ptr<TimerBase> pTimer = iter->second;
			this->mTimerMap.erase(iter);
			return true;
		}
		return false;
	}

	shared_ptr<TimerBase> TimerManager::GetTimer(long long id)
	{
		auto iter = this->mTimerMap.find(id);
		return iter != this->mTimerMap.end() ? iter->second : nullptr;
	}

	void TimerManager::OnSystemUpdate()
	{
		this->mStartTime = TimeHelper::GetMilTimestamp();
		const long long nowTime = TimeHelper::GetMilTimestamp();

		//shared_ptr<TimerBase> newTimer;

		if (this->mTickCount >= this->mMaxTimeCount)
		{
			auto iter = this->mNextWheelTimer.begin();
			for (; iter != this->mNextWheelTimer.end();)
			{
				(*iter)->mTickCount -= this->mMaxTimeCount;
				if (this->AddNextTimer((*iter)))
				{
					this->mNextWheelTimer.erase(iter++);
					continue;
				}
				iter++;
			}
			this->mTickCount = 0;
			this->mStartTime = nowTime;
		}
		int tickCount = (nowTime - mStartTime) / TimerPrecision;
		for (int index = this->mTickCount; index <= tickCount && index < this->mMaxTimeCount; index++)
		{
			std::queue<shared_ptr<TimerBase>> &taskList = this->mTimers[index];
			while (!taskList.empty())
			{
				shared_ptr<TimerBase> timerData = taskList.front();

				this->InvokeTimer(timerData->GetTimerId());

				taskList.pop();
			}
		}
		this->mTickCount = tickCount;
	}

	bool TimerManager::InvokeTimer(long long id)
	{
		auto iter = this->mTimerMap.find(id);
		if (iter != this->mTimerMap.end())
		{
			iter->second->Invoke();
			this->mTimerMap.erase(iter);
			return true;
		}
		return false;
	}

	bool TimerManager::AddNextTimer(shared_ptr<TimerBase> timer)
	{
		if (timer->mTickCount < this->mMaxTimeCount)
		{
			this->mTimerCount++;
			this->mTimers[timer->mTickCount].push(timer);
			return true;
		}
		return false;
	}

	void TimerManager::AddTimerToWheel(shared_ptr<TimerBase> timer)
	{
		int solt = timer->mDelayTime / TimerPrecision;
		timer->mTickCount = solt + this->mTickCount;
		if (timer->mTickCount < this->mMaxTimeCount)
		{
			this->mTimers[timer->mTickCount].push(timer);
		}
		else if (timer->mTickCount - this->mMaxTimeCount < this->mMaxTimeCount)
		{
			timer->mTickCount -= this->mMaxTimeCount;
			this->mTimers[timer->mTickCount].push(timer);
		}
		else
		{
			this->mNextWheelTimer.push_back(timer);
		}
	}
}