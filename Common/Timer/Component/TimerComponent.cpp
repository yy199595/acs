#include"TimerComponent.h"
#include"Timer/Timer/DelayTimer.h"
#include"Entity/Actor/App.h"
#include"Lua/Engine/ModuleClass.h"
#include"Timer/Lua/Timer.h"
namespace acs
{
	TimerComponent::TimerComponent()
	{
		this->mNextUpdateTime = 0;
	}

	bool TimerComponent::Awake()
	{
		for (int index = 0; index < this->LayerCount; index++)
		{
			int count = index == 0
						? this->FirstLayerCount : this->OtherLayerCount;
			int end = FirstLayerCount * (int)std::pow(OtherLayerCount, index);

			int start = index == 0 ? 0 :
						FirstLayerCount * (int)std::pow(OtherLayerCount, index - 1);
			this->mTimerLayers.push_back(new TimeWheelLayer(index, count, start, end));
		}
		return true;
	}

	bool TimerComponent::AddTimer(std::unique_ptr<TimerBase> timer)
	{
		return this->AddTimerToWheel(std::move(timer));
	}

	void TimerComponent::AddUpdateTimer(std::unique_ptr<TimerBase> timer)
	{
		if (timer == nullptr)
		{
			return;
		}
		this->mUpdateTimer.emplace_back(timer->GetTimerId());
		this->mTimerMap.emplace(timer->mTimerId, std::move(timer));
	}

	long long TimerComponent::CreateTimer(unsigned int ms, StaticMethod* func)
	{
		std::unique_ptr<TimerBase> timerBase(new DelayTimer(ms, func));
		if (ms == 0)
		{
			this->mLastFrameTriggerTimers.push(timerBase->mTimerId);
			this->mTimerMap.emplace(timerBase->mTimerId, std::move(timerBase));
			return 0;
		}
		long long id = timerBase->GetTimerId();
		return this->AddTimer(std::move(timerBase)) ? id : 0;
	}

	void TimerComponent::OnLuaRegister(Lua::ModuleClass &luaRegister)
	{
		luaRegister.AddFunction("Add", Lua::Timer::Add);
		luaRegister.AddFunction("Del", Lua::Timer::Remove);
		luaRegister.AddFunction("AddUpdate", Lua::Timer::AddUpdate);
		luaRegister.End("core.timer");
	}

	bool TimerComponent::CancelTimer(long long id)
	{
		auto iter = this->mTimerMap.find(id);
		if (iter != this->mTimerMap.end())
		{
            this->mTimerMap.erase(iter);
			return true;
		}
		return false;
	}

	void TimerComponent::OnSystemUpdate()
	{
		if (this->mNextUpdateTime == 0)
		{
			this->mNextUpdateTime = help::Time::NowMil();
			return;
		}
		long long nowTime = help::Time::NowMil();
		long long subTime = nowTime - this->mNextUpdateTime;
		const int tick = (int)subTime / this->TimerPrecision;

		if (tick <= 0) return;

		this->mNextUpdateTime = nowTime - (subTime % this->TimerPrecision);

		for (int count = 0; count < tick; count++)
		{
			for (auto & timeWheelLayer : this->mTimerLayers)
			{
				std::queue<long long>& timerQueue = timeWheelLayer->GetTimerQueue();
				if (timeWheelLayer->GetLayerId() == 0)
				{
					while (!timerQueue.empty())
					{
						long long id = timerQueue.front();
						this->InvokeTimer(id);
						timerQueue.pop();
					}
				}
				else
				{
					while (!timerQueue.empty())
					{
						long long id = timerQueue.front();
						this->AddTimerToWheel(id);
						timerQueue.pop();
					}
				}
				if (!timeWheelLayer->JumpNextLayer())
				{
					break;
				}
			}
		}
	}

	void TimerComponent::OnFrameUpdate(long long nowTime)
	{
		while(!this->mLastFrameTriggerTimers.empty())
		{
			long long timerId = this->mLastFrameTriggerTimers.front();
			{
				this->InvokeTimer(timerId);
			}
			this->mLastFrameTriggerTimers.pop();
		}
		if(!this->mUpdateTimer.empty())
		{
			for (auto iter = this->mUpdateTimer.begin(); iter != this->mUpdateTimer.end();)
			{
				auto item = this->mTimerMap.find(*iter);
				if (item == this->mTimerMap.end())
				{
					this->mUpdateTimer.erase(iter++);
					continue;
				}
				if (nowTime >= item->second->GetTargetTime())
				{
					item->second->Invoke();
					item->second->UpdateTargetTime();
				}
				iter++;
			}
		}
	}

	bool TimerComponent::InvokeTimer(long long timerId)
	{
		auto iter = this->mTimerMap.find(timerId);
		if (iter == this->mTimerMap.end())
		{
			return false;
		}
		iter->second->Invoke();
		this->mTimerMap.erase(iter);
		return true;
	}

	bool TimerComponent::AddTimerToWheel(long long timerId)
	{
		auto iter = this->mTimerMap.find(timerId);
		if (iter == this->mTimerMap.end())
		{
			return false;
		}
		std::unique_ptr<TimerBase> timerBase = std::move(iter->second);
		{
			this->mTimerMap.erase(iter);
		}
		return this->AddTimerToWheel(std::move(timerBase));
	}

	bool TimerComponent::AddTimerToWheel(std::unique_ptr<TimerBase> timer)
	{
		long long nowTime = help::Time::NowMil();
		long long useTime = timer->GetTargetTime() - nowTime;
		int tick = (int)useTime / this->TimerPrecision;
		if (tick <= 0)
		{
			this->mLastFrameTriggerTimers.push(timer->mTimerId);
			this->mTimerMap.emplace(timer->mTimerId, std::move(timer));
			return true;
		}
		for (auto timerLayer : this->mTimerLayers)
		{
			if (timerLayer->AddTimer(tick, timer->mTimerId))
			{
				this->mTimerMap.emplace(timer->mTimerId, std::move(timer));
				return true;
			}
		}
		LOG_ERROR("add timer failure id {} ", timer->GetTimerId());
		return false;
	}

	long long TimerComponent::DelayCall(int ms, std::function<void()>&& callback)
	{
		return this->CreateTimer(ms, new LambdaMethod(std::move(callback)));
	}
}// namespace Sentry