#include "TimerComponent.h"
#include <Timer/DelayTimer.h>

namespace GameKeeper
{
    bool TimerComponent::Awake()
    {
        for (int index = 0; index < this->LayerCount; index++)
        {
            int count = index == 0
                    ? this->FirstLayerCount : this->OtherLayerCount;
            int end = FirstLayerCount * std::pow(OtherLayerCount, index);

            int start = index == 0 ? 0 :
                    FirstLayerCount * std::pow(OtherLayerCount, index - 1);
            this->mTimerLayers.push_back(new TimeWheelLayer(index, count, start, end));
        }
        this->mNextUpdateTime = TimeHelper::GetMilTimestamp() + this->TimerPrecision;
        return true;
    }

    bool TimerComponent::AddTimer(TimerBase * timer)
    {
        if (timer == nullptr)
        {
            return false;
        }

        long long id = timer->GetTimerId();
        auto iter = this->mTimerMap.find(id);
        if (iter == this->mTimerMap.end())
        {
            this->mTimerMap.emplace(id, timer);
            return this->AddTimerToWheel(timer);
        }
        return false;
    }

    bool TimerComponent::AddTimer(long long ms, StaticMethod * func)
    {
        if (ms == 0)
        {
            func->run();
			delete func;
            return true;
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
        long long nowTime = TimeHelper::GetMilTimestamp();
        long long subTime = nowTime - this->mNextUpdateTime;

        if (subTime <= (this->TimerPrecision - 2)) //2毫秒误差
        {
            return;
        }
        int count = subTime / this->TimerPrecision;
        count = count == 0 ? 1 : count;

        this->mNextUpdateTime = nowTime + this->TimerPrecision - subTime;

        for (int index = 0; index < count; index++)
        {
            TimeWheelLayer *timerLayer = this->mTimerLayers[0];

            bool res = timerLayer->MoveIndex(this->mTimers);
            while (!this->mTimers.empty())
            {
                auto timer = this->mTimers.front();
                this->mTimers.pop();
                this->InvokeTimer(timer->GetTimerId());
            }
            for (size_t i = 1; i < this->mTimerLayers.size() && res; i++)
            {
                timerLayer = this->mTimerLayers[i];
                res = timerLayer->MoveIndex(this->mTimers);

                while (!this->mTimers.empty())
                {
                    auto timer = this->mTimers.front();
                    this->mTimers.pop();
                    this->AddTimerToWheel(timer);
                }
            }
        }
    }

    bool TimerComponent::InvokeTimer(long long id)
    {
        auto iter = this->mTimerMap.find(id);
        if (iter != this->mTimerMap.end())
        {
            TimerBase * timer = iter->second;
            if(timer->Invoke())
            {
                delete timer;
                this->mTimerMap.erase(iter);
                return true;
            }
            return this->AddTimerToWheel(timer);
        }
        return false;
    }

    bool TimerComponent::AddTimerToWheel(TimerBase * timer)
    {
        auto iter = this->mTimerMap.find(timer->GetTimerId());
        if (iter == this->mTimerMap.end())
        {
            GKDebugError("add timer not exist : " << timer->GetTimerId());
            return false;
        }
        long long nowTime = TimeHelper::GetMilTimestamp();
        int tick = (timer->GetTriggerTime() - nowTime) / this->TimerPrecision;
        for (auto timerLayer : this->mTimerLayers)
        {
            if (timerLayer->AddTimer(tick, timer))
            {
                return true;
            }
        }
        GKDebugError("add timer " << timer->GetTimerId() << " failure");
        return false;
    }
}// namespace GameKeeper