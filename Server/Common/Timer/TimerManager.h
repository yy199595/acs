#pragma once

#include "TimeWheelLayer.h"
#include <Manager/Manager.h>
#include <Util/NumberBuilder.h>

namespace Sentry
{
    class TimerManager : public Manager, public ISystemUpdate
    {
    public:
        TimerManager();

        ~TimerManager() {}

    public:
        bool RemoveTimer(long long id);

        bool AddTimer(shared_ptr<TimerBase> timer);

        bool AddTimer(long long ms, std::function<void(void)> func);

        shared_ptr<TimerBase> GetTimer(long long id);

    public:
        template<typename T, typename... Args>
        shared_ptr<T> CreateTimer(Args &&...args);

    protected:
        bool OnInit();

        void OnSystemUpdate() final; //处理系统事件
    private:
        void RefreshTimer();

        bool InvokeTimer(long long id);

        bool AddTimerToWheel(shared_ptr<TimerBase> timer);

    private:
        const int LayerCount = 5;
        const int TimerPrecision = 20;
        const int OtherLayerCount = 32;
        const int FirstLayerCount = 256;

    private:
        long long mNextUpdateTime;
        NumberBuilder<long long> mTimerIdPool;
        std::queue<shared_ptr<TimerBase>> mTimers;
        std::vector<TimeWheelLayer *> mTimerLayers;
        std::unordered_map<long long, shared_ptr<TimerBase>> mTimerMap; //所有timer的列表
    };

    template<typename T, typename... Args>
    inline shared_ptr<T> TimerManager::CreateTimer(Args &&...args)
    {
        std::shared_ptr<T> timer = std::make_shared<T>(std::forward<Args>(args)...);
        return this->AddTimer(timer) ? timer : nullptr;
    }
}