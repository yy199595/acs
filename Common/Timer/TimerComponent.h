#pragma once

#include "TimeWheelLayer.h"
#include <Component/Component.h>
#include <Util/NumberBuilder.h>
#include <Method/MethodProxy.h>
namespace Sentry
{
    class TimerComponent : public Component, public ISystemUpdate
    {
    public:
        TimerComponent();

        ~TimerComponent() {}

    public:
        bool RemoveTimer(long long id);

        bool AddTimer(shared_ptr<TimerBase> timer);

		template<typename F,typename O, typename ... Args>
		bool AddTimer(long long ms, F && f, O * o, Args &&... args) {

			StaticMethod * methodProxy = NewMethodProxy(
				std::forward<F>(f), o, std::forward<Args>(args)...);
			return this->AddTimer(ms, methodProxy);
		}

        shared_ptr<TimerBase> GetTimer(long long id);
	private:
		bool AddTimer(long long ms, StaticMethod * func);

    public:
        template<typename T, typename... Args>
        shared_ptr<T> CreateTimer(Args &&...args);

    protected:
        bool Awake();

        void OnSystemUpdate() final;//处理系统事件

        bool InvokeTimer(long long id);

        bool AddTimerToWheel(shared_ptr<TimerBase> timer);

		int GetPriority() override { return 1; }
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
        std::unordered_map<long long, shared_ptr<TimerBase>> mTimerMap;//所有timer的列表
    };

    template<typename T, typename... Args>
    inline shared_ptr<T> TimerComponent::CreateTimer(Args &&...args)
    {
        std::shared_ptr<T> timer = std::make_shared<T>(std::forward<Args>(args)...);
        return this->AddTimer(timer) ? timer : nullptr;
    }
}// namespace Sentry