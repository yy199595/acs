#include "TimerManager.h"
#include<Util/TimeHelper.h>
namespace SoEasy
{
	TimerManager::TimerManager()
	{
		this->mIsStop = false;
		this->mTickCount = 0;
		this->mTimerCount = 0;	
		this->mTimerThread = nullptr;
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
			this->mWheelQueue.AddItem(timer);
			this->mTimerMap.emplace(id, timer);
			return true;
		}	
		return false;
	}

	

	void TimerManager::Stop()
	{
		this->mIsStop = true;
		this->mWheelVariable.notify_one();
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

	void TimerManager::OnInitComplete()
	{
		if (this->mTimerThread == nullptr)
		{
			this->mTimerThread = new std::thread(&TimerManager::RefreshTimer, this);
			if (this->mTimerThread != nullptr)
			{
				this->mTimerThread->detach();
			}		
		}
	}

	void TimerManager::OnSystemUpdate()
	{
		long long timerId = 0;
		this->mFinishTimerQueue.SwapQueueData();
		if (this->mFinishTimerQueue.PopItem(timerId))
		{
			auto iter = this->mTimerMap.find(timerId);
			if (iter != this->mTimerMap.end())
			{
				shared_ptr<TimerBase> timer = iter->second;
				if (timer->Invoke() == false)
				{
					this->mWheelQueue.AddItem(timer);
					return;
				}
				this->mTimerMap.erase(iter);
			}
		}
	}

	void TimerManager::RefreshTimer()
	{
		this->mStartTime = TimeHelper::GetMilTimestamp();
		while (this->mIsStop == false)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(TimerPrecision));
			const long long nowTime = TimeHelper::GetMilTimestamp();

			shared_ptr<TimerBase> newTimer;
			this->mWheelQueue.SwapQueueData();
			while (this->mWheelQueue.PopItem(newTimer))
			{
				this->AddTimerToWheel(newTimer);
			}

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
				std::queue<shared_ptr<TimerBase>> & taskList = this->mTimers[index];
				while (!taskList.empty())
				{
					shared_ptr<TimerBase> timerData = taskList.front();
					this->mFinishTimerQueue.AddItem(timerData->GetTimerId());
					taskList.pop();
				}
			}
			this->mTickCount = tickCount;
		}
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