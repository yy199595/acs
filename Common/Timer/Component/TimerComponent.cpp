#include"TimerComponent.h"
#include"Timer/Timer/DelayTimer.h"
#include"Lua/Engine/ModuleClass.h"
#include "Lua/Lib/Lib.h"
#include "Entity/Actor/App.h"

namespace config
{
	constexpr int LayerCount = 3;  //时间轮的层数
	constexpr int TimerPrecision = 100; //定时器精度 100就是0.1s, 50就是0.05s
	constexpr int OtherLayerCount = 60;
	constexpr int FirstLayerCount = 60 * 1000 / TimerPrecision;
}

namespace acs
{
	TimerComponent::TimerComponent()
	{
		this->mDoneCount = 0;
		this->mCancelCount = 0;
		this->mNextUpdateTime = 0;
		this->mTimerMap.reserve(100);
		this->mTimerMap.max_load_factor(0.75);
	}

	bool TimerComponent::Awake()
	{
		LuaCCModuleRegister::Add([](Lua::CCModule & ccModule) {
			ccModule.Open("core.timer", lua::lib::luaopen_ltimer);
		});

		for (int index = 0; index < config::LayerCount; index++)
		{
			int count = index == 0
						? config::FirstLayerCount : config::OtherLayerCount;
			unsigned int end = config::FirstLayerCount * (unsigned int)std::pow(config::OtherLayerCount, index);

			unsigned int start = index == 0 ? 0 :
						config::FirstLayerCount * (unsigned int)std::pow(config::OtherLayerCount, index - 1);
			this->mTimerLayers.emplace_back(std::make_unique<TimeWheelLayer>(index + 1, count, start, end));

			help::Time::Date timeData2 = help::Time::CalcHourMinSecond(end * config::TimerPrecision);
			help::Time::Date timeData1 = help::Time::CalcHourMinSecond(start * config::TimerPrecision);

//			LOG_DEBUG("layer:{} min:[{}:{}:{}] max:[{}:{}:{}]", index + 1, timeData1.Hour + timeData1.Day * 24,
//					timeData1.Minute, timeData1.Second, timeData2.Hour + timeData2.Day * 24, timeData2.Minute, timeData2.Second);
		}
		long long nowTime = help::Time::NowSec();
		this->mNextUpdateTime = help::Time::NowMil();
		long long diffTime = help::Time::GetNewTime(1) - nowTime;
		this->Timeout((int)(diffTime * 1000), &TimerComponent::OnNewDay, this);
		return true;
	}

	void TimerComponent::OnNewDay()
	{
		std::vector<ISystemNewDay*> newDayComponents;
		this->mApp->GetComponents(newDayComponents);
		for(ISystemNewDay * systemNewDay : newDayComponents)
		{
			systemNewDay->OnNewDay();
		}
		long long nowTime = help::Time::NowSec();
		long long diffTime = help::Time::GetNewTime(1) - nowTime;
		this->Timeout((unsigned int)(diffTime * 1000), &TimerComponent::OnNewDay, this);
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

	long long TimerComponent::CreateTimer(unsigned int ms, std::unique_ptr<StaticMethod> func)
	{
		std::unique_ptr<TimerBase> timerBase = std::make_unique<DelayTimer>(ms, std::move(func));
		if (ms == 0)
		{
			this->mLastFrameTriggerTimers.emplace(timerBase->mTimerId);
			this->mTimerMap.emplace(timerBase->mTimerId, std::move(timerBase));
			return 0;
		}
		long long id = timerBase->GetTimerId();
		return this->AddTimer(std::move(timerBase)) ? id : 0;
	}

	bool TimerComponent::CancelTimer(long long id)
	{
		auto iter = this->mTimerMap.find(id);
		if (iter == this->mTimerMap.end())
		{
			return false;
		}
		this->mCancelCount++;
		this->mTimerMap.erase(iter);
		return true;
	}

	void TimerComponent::OnSystemUpdate() noexcept
	{
		if (this->mNextUpdateTime == 0)
		{
			return;
		}
		long long nowTime = help::Time::NowMil();
		long long subTime = nowTime - this->mNextUpdateTime;
		const int tick = (int)subTime / config::TimerPrecision;

		if (tick <= 0) return;

		this->mNextUpdateTime = nowTime - (subTime % config::TimerPrecision);

		for (int count = 0; count < tick; count++)
		{
			std::unique_ptr<TimeWheelLayer> & timeWheelLayer = this->mTimerLayers.front();
			{
				std::queue<long long>& timerQueue = timeWheelLayer->GetTimerQueue();
				while (!timerQueue.empty())
				{
					long long id = timerQueue.front();
					this->InvokeTimer(id);
					timerQueue.pop();
				}
			}
			if(timeWheelLayer->JumpNextLayer())
			{
				for(size_t index = 1; index < this->mTimerLayers.size(); index++)
				{
					std::unique_ptr<TimeWheelLayer> & wheelLayer = this->mTimerLayers.at(index);
					{
						std::queue<long long>& timerQueue = wheelLayer->GetTimerQueue();
						{
							while(!timerQueue.empty())
							{
								this->AddTimerToWheel(timerQueue.front());
								timerQueue.pop();
							}
						}
						if(!wheelLayer->JumpNextLayer())
						{
							break;
						}
					}
				}
			}
		}
	}

	void TimerComponent::OnRecord(json::w::Document& document)
	{
		std::unique_ptr<json::w::Value> jsonValue = document.AddObject("timer");
		{
			jsonValue->Add("wait", this->mTimerMap.size());
			jsonValue->Add("done", this->mDoneCount);
			jsonValue->Add("cancel", this->mCancelCount);
		}
	}

	void TimerComponent::OnFrameUpdate(long long nowTime) noexcept
	{
		while (!this->mLastFrameTriggerTimers.empty())
		{
			long long timerId = this->mLastFrameTriggerTimers.front();
			{
				this->InvokeTimer(timerId);
			}
			this->mLastFrameTriggerTimers.pop();
		}

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
				item->second->Refresh();
				item->second->Invoke();
			}
			iter++;
		}
	}

	bool TimerComponent::InvokeTimer(long long timerId)
	{
		auto iter = this->mTimerMap.find(timerId);
		if (iter == this->mTimerMap.end())
		{
			return false;
		}
		this->mDoneCount++;
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
		long long nowTime = help::Time::NowMil();
		std::unique_ptr<TimerBase> & timer = iter->second;
		long long useTime = timer->GetTargetTime() - nowTime;
		unsigned int tick = (unsigned int)useTime / config::TimerPrecision;
		if (tick <= 0)
		{
			this->mLastFrameTriggerTimers.push(timer->mTimerId);
			return true;
		}
		for (std::unique_ptr<TimeWheelLayer> & timerLayer : this->mTimerLayers)
		{
			if (timerLayer->AddTimer(tick, timer->mTimerId))
			{
				return true;
			}
		}
		LOG_ERROR("add timer failure id {} ", timer->GetTimerId());
		return false;
	}

	bool TimerComponent::AddTimerToWheel(std::unique_ptr<TimerBase> timer)
	{
		long long nowTime = help::Time::NowMil();
		long long useTime = timer->GetTargetTime() - nowTime;
//		help::Time::Date timeDate = help::Time::CalcHourMinSecond(useTime);
//		CONSOLE_LOG_WARN("run in => {}:{}:{}", timeDate.Hour + timeDate.Day * 24, timeDate.Minute, timeDate.Second);
		unsigned int tick = (unsigned int)useTime / config::TimerPrecision;
		if (tick <= 0)
		{
			this->mLastFrameTriggerTimers.push(timer->mTimerId);
			this->mTimerMap.emplace(timer->mTimerId, std::move(timer));
			return true;
		}
		for (std::unique_ptr<TimeWheelLayer> & timerLayer : this->mTimerLayers)
		{
			if (timerLayer->AddTimer(tick, timer->mTimerId))
			{
				this->mTimerMap.emplace(timer->mTimerId, std::move(timer));
				//CONSOLE_LOG_INFO("[{}] ({}) count={}", timerLayer->GetLayerId(), tick, this->mTimerMap.size())
				return true;
			}
		}
		LOG_ERROR("add timer failure id {} ", timer->GetTimerId());
		return false;
	}

	long long TimerComponent::Timeout(unsigned int ms, std::function<void()>&& callback)
	{
		return this->CreateTimer(ms, std::make_unique<LambdaMethod>(std::move(callback)));
	}
}// namespace Sentry