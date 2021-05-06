#include "TimerManager.h"
#include<Util/TimeHelper.h>
namespace SoEasy
{
	TimerManager::TimerManager()
	{
		this->mIsStop = false;
		this->mWheelInterval = 20;
		this->mTimerThread = nullptr;
	}

	bool TimerManager::AddTimer(shared_ptr<TimerBase> timer)
	{
		if (timer == nullptr)
		{
			return false;
		}
		long long id = timer->GetTimerId();
		auto iter = this->mTimerMap.find(id);
		if (iter != this->mTimerMap.end())
		{
			return false;
		}
		this->mTimerThreadLock.lock();
		this->mNextWheelQueue.push(timer);
		this->mTimerThreadLock.unlock();
		this->mWheelVariable.notify_one();
		this->mTimerMap.insert(std::make_pair(id, timer));
		return true;
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

	void TimerManager::OnTaskFinish(long long id)
	{
		auto iter = this->mTimerMap.find(id);
		if (iter != this->mTimerMap.end())
		{
			shared_ptr<TimerBase> timer = iter->second;
			if (timer->Invoke() == false)
			{
				this->AddTimer(timer);
			}
			this->mTimerMap.erase(iter);
		}
	}

	void TimerManager::RefreshTimer()
	{
		while (this->mIsStop == false)
		{
			if (this->mNextWheelQueue.empty())
			{
				std::unique_lock<std::mutex> lck(this->mTimerThreadLock);
				this->mWheelVariable.wait(lck);
			}
			else
			{
				long long startTime = TimeHelper::GetMilTimestamp();
				std::swap(this->mNextWheelQueue, this->mWheelQueue);
				while (!this->mWheelQueue.empty())
				{
					shared_ptr<TimerBase> timer = this->mWheelQueue.front();
					this->mWheelQueue.pop();
					if (timer->IsTrigger(startTime))
					{
						long long id = timer->GetTimerId();
						this->AddFinishTaskId(id);
						continue;
					}
					this->mNextWheelQueue.push(timer);
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(this->mWheelInterval));
			}
		}
	}
}