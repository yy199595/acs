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
		if (iter == this->mTimerMap.end())
		{
			this->mTimerMap.insert(std::make_pair(id, timer));
			this->mTimerThreadLock.lock();
			this->mNextWheelQueue.push(timer);
			this->mTimerThreadLock.unlock();
			this->mWheelVariable.notify_one();
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
					this->mTimerThreadLock.lock();
					this->mNextWheelQueue.push(timer);
					this->mTimerThreadLock.unlock();
					this->mWheelVariable.notify_one();
					return;
				}
				this->mTimerMap.erase(iter);
			}
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
			long long startTime = TimeHelper::GetMilTimestamp();

			this->mTimerThreadLock.lock();
			std::swap(this->mNextWheelQueue, this->mWheelQueue);
			this->mTimerThreadLock.unlock();

			while (!this->mWheelQueue.empty())
			{
				this->mTimerThreadLock.lock();
				shared_ptr<TimerBase> timer = this->mWheelQueue.front();
				this->mWheelQueue.pop();
				this->mTimerThreadLock.unlock();

				if (timer->IsTrigger(startTime))
				{
					long long id = timer->GetTimerId();
					this->mFinishTimerQueue.AddItem(id);
					continue;
				}
				this->mTimerThreadLock.lock();
				this->mNextWheelQueue.push(timer);
				this->mTimerThreadLock.unlock();
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(this->mWheelInterval));
		}
	}
}