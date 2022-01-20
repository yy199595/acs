#pragma once

#include"TimeWheelLayer.h"
#include<Component/Component.h>
#include<Method/MethodProxy.h>
namespace Sentry
{
    class TimerComponent : public Component, public ISystemUpdate
    {
    public:
        TimerComponent() = default;

        ~TimerComponent() override = default;

    public:
        bool RemoveTimer(long long id);

		template<typename F,typename O, typename ... Args>
		unsigned int AsyncWait(unsigned int ms, F && f, O * o, Args &&... args) {

			StaticMethod * methodProxy = NewMethodProxy(
				std::forward<F>(f), o, std::forward<Args>(args)...);
			return this->AddTimer(ms, methodProxy);
		}
        TimerBase * GetTimer(long long id);
        long long AddTimer(TimerBase * timer);
		long long AddTimer(unsigned int ms, StaticMethod * func);

    public:
        template<typename T, typename... Args>
        T * CreateTimer(Args &&...args);

    protected:
        bool Awake() final;

		bool LateAwake() final;

        void OnSystemUpdate() final;//处理系统事件

        bool AddTimerToWheel(TimerBase * timer);

    private:
        const int LayerCount = 5;
        const int TimerPrecision = 20;
        const int OtherLayerCount = 32;
        const int FirstLayerCount = 256;
    private:
        long long mNextUpdateTime;
        std::queue<long long> mTimers;
        std::vector<TimeWheelLayer *> mTimerLayers;
        std::unordered_map<long long, TimerBase *> mTimerMap;//所有timer的列表
    };

    template<typename T, typename... Args>
    inline T * TimerComponent::CreateTimer(Args &&...args)
    {
        return new T(std::forward<Args>(args)...);
    }
}// namespace Sentry