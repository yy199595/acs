#pragma once

#include "TimeWheelLayer.h"
#include <Component/Component.h>
#include <Util/NumberBuilder.h>
#include <Method/MethodProxy.h>
namespace GameKeeper
{
    class TimerComponent : public Component, public ISystemUpdate
    {
    public:
        TimerComponent() = default;

        ~TimerComponent() override = default;

    public:
        bool RemoveTimer(long long id);

        bool AddTimer(TimerBase * timer);

		template<typename F,typename O, typename ... Args>
		bool AddTimer(long long ms, F && f, O * o, Args &&... args) {

			StaticMethod * methodProxy = NewMethodProxy(
				std::forward<F>(f), o, std::forward<Args>(args)...);
			return this->AddTimer(ms, methodProxy);
		}

        TimerBase * GetTimer(long long id);
	private:
		bool AddTimer(long long ms, StaticMethod * func);

    public:
        template<typename T, typename... Args>
        T * CreateTimer(Args &&...args);

    protected:
        bool Awake() final;

        void OnSystemUpdate() final;//处理系统事件

        bool InvokeTimer(long long id);

        bool AddTimerToWheel(TimerBase * timer);

		int GetPriority() override { return 1; }
    private:
        const int LayerCount = 5;
        const int TimerPrecision = 20;
        const int OtherLayerCount = 32;
        const int FirstLayerCount = 256;

    private:
        long long mNextUpdateTime;
        std::queue<TimerBase *> mTimers;
        NumberBuilder<long long> mTimerIdPool;
        std::vector<TimeWheelLayer *> mTimerLayers;
        std::unordered_map<long long, TimerBase *> mTimerMap;//所有timer的列表
    };

    template<typename T, typename... Args>
    inline T * TimerComponent::CreateTimer(Args &&...args)
    {
        return new T(std::forward<Args>(args)...);
    }
}// namespace GameKeeper