﻿#include "TimerComponent.h"
#include <Timer/DelayTimer.h>

namespace Sentry
{
    TimerComponent::TimerComponent()
    {
    }

    bool TimerComponent::Awake()
    {
        for (int index = 0; index < this->LayerCount; index++)
        {
            int count = index == 0 ? this->FirstLayerCount : this->OtherLayerCount;
            int end = FirstLayerCount * std::pow(OtherLayerCount, index);
            int start = index == 0 ? 0 : FirstLayerCount * std::pow(OtherLayerCount, index - 1);
            TimeWheelLayer *timerLayer = new TimeWheelLayer(index, count, start, end);
            this->mTimerLayers.push_back(timerLayer);
        }
        this->mNextUpdateTime = TimeHelper::GetMilTimestamp() + this->TimerPrecision;
        return true;
    }

    bool TimerComponent::AddTimer(shared_ptr<TimerBase> timer)
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
            this->AddTimerToWheel(timer);
            return true;
        }
        return false;
    }

    bool TimerComponent::AddTimer(long long ms, MethodProxy * func)
    {
        if (ms == 0)
        {
            func->run();
			delete func;
            return true;
        }
        return this->AddTimer(make_shared<DelayTimer>(ms, func));
    }

    bool TimerComponent::RemoveTimer(long long id)
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

    shared_ptr<TimerBase> TimerComponent::GetTimer(long long id)
    {
        auto iter = this->mTimerMap.find(id);
        return iter != this->mTimerMap.end() ? iter->second : nullptr;
    }

    void TimerComponent::OnSystemUpdate()
    {
        long long nowTime = TimeHelper::GetMilTimestamp();
        long long subTime = nowTime - this->mNextUpdateTime;

        if (subTime <= (this->TimerPrecision - 2))
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
            iter->second->Invoke();
            this->mTimerMap.erase(iter);
            return true;
        }
        return false;
    }

    bool TimerComponent::AddTimerToWheel(shared_ptr<TimerBase> timer)
    {
        auto iter = this->mTimerMap.find(timer->GetTimerId());
        if (iter == this->mTimerMap.end())
        {
            return false;
        }
        long long nowTime = TimeHelper::GetMilTimestamp();

        int tick = (timer->GetTriggerTime() - nowTime) / this->TimerPrecision;
        for (size_t index = 0; index < this->mTimerLayers.size(); index++)
        {
            TimeWheelLayer *timerLayer = this->mTimerLayers[index];
            if (timerLayer->AddTimer(tick, timer))
            {
                return true;
            }
        }
        return false;
    }
}// namespace Sentry