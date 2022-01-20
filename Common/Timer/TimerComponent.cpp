#include "TimerComponent.h"
#include <Timer/DelayTimer.h>
#include"Core/App.h"
namespace Sentry
{
    bool TimerComponent::Awake()
    {
		this->mNextUpdateTime = 0;
        for (int index = 0; index < this->LayerCount; index++)
        {
            int count = index == 0
                    ? this->FirstLayerCount : this->OtherLayerCount;
            int end = FirstLayerCount * std::pow(OtherLayerCount, index);

            int start = index == 0 ? 0 :
                    FirstLayerCount * std::pow(OtherLayerCount, index - 1);
            this->mTimerLayers.push_back(new TimeWheelLayer(index, count, start, end));
        }
        
        return true;
    }

	bool TimerComponent::LateAwake()
    {
        this->mNextUpdateTime = Helper::Time::GetMilTimestamp();
        return true;
    }

    long long TimerComponent::AddTimer(TimerBase * timer)
    {
        LOG_CHECK_FATAL(this->mNextUpdateTime > 0);
        if (timer == nullptr || !this->AddTimerToWheel(timer))
        {
            return 0;
        }
        return timer->GetTimerId();
    }

    long long TimerComponent::AddTimer(unsigned int ms, StaticMethod * func)
    {
        if (ms == 0)
        {
            func->run();
			delete func;
            return 0;
        }
        return this->AddTimer(new DelayTimer(ms, func));
    }

    bool TimerComponent::RemoveTimer(long long id)
    {
        auto iter = this->mTimerMap.find(id);
        if (iter != this->mTimerMap.end())
        {
            delete iter->second;
            this->mTimerMap.erase(iter);
            return true;
        }
        return false;
    }

    TimerBase * TimerComponent::GetTimer(long long id)
    {
        auto iter = this->mTimerMap.find(id);
        return iter != this->mTimerMap.end() ? iter->second : nullptr;
    }

    void TimerComponent::OnSystemUpdate()
    {
        long long nowTime = Helper::Time::GetMilTimestamp();
        long long subTime = nowTime - this->mNextUpdateTime;
        const int tick = subTime / this->TimerPrecision;

        if(tick <= 0) return;


        this->mNextUpdateTime = nowTime - (subTime % this->TimerPrecision);
        for (int index = 0; index < tick; index++)
        {
            TimeWheelLayer *timerLayer = this->mTimerLayers[0];
            bool res = timerLayer->MoveIndex(this->mTimers);
            while (!this->mTimers.empty())
            {
                auto id = this->mTimers.front();
                this->mTimers.pop();
                auto iter = this->mTimerMap.find(id);
                if (iter != this->mTimerMap.end())
                {
                    auto timer = iter->second;
                    this->mTimerMap.erase(iter);
                    if(timer->Invoke())
                    {
                        delete timer;
                        continue;
                    }
                    this->AddTimerToWheel(timer);
                }
            }
            for (size_t i = 1; i < this->mTimerLayers.size() && res; i++)
            {
                timerLayer = this->mTimerLayers[i];
                res = timerLayer->MoveIndex(this->mTimers);

                while (!this->mTimers.empty())
                {
                    auto id = this->mTimers.front();
                    this->mTimers.pop();
                    auto timer = this->GetTimer(id);
                    if(timer != nullptr)
                    {
                        this->AddTimerToWheel(timer);
                    }
                }
            }
        }
    }

    bool TimerComponent::AddTimerToWheel(TimerBase * timer)
    {
        long long nowTime = Helper::Time::GetMilTimestamp();
        int tick = (timer->GetTargetTime() - nowTime) / this->TimerPrecision;
        for (auto timerLayer : this->mTimerLayers)
        {
            if (timerLayer->AddTimer(tick, timer->mTimerId))
            {
                this->mTimerMap.emplace(timer->mTimerId, timer);
                return true;
            }
        }
        delete timer;
        LOG_ERROR("add timer {0} failure", timer->GetTimerId());
        return false;
    }
}// namespace Sentry