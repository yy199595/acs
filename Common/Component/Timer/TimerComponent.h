#pragma once

#include"Timer/TimeWheelLayer.h"
#include"Component/Component.h"
#include"Method/MethodProxy.h"
namespace Sentry
{
	class TimerComponent : public Component, public ISystemUpdate
	{
	 public:
		TimerComponent() = default;

		~TimerComponent() override = default;

	 public:
		bool CancelTimer(long long id);

		template<typename F, typename O, typename ... Args>
		unsigned int AsyncWait(unsigned int ms, F&& f, O* o, Args&& ... args)
		{
			StaticMethod* methodProxy = NewMethodProxy(
				std::forward<F>(f), o, std::forward<Args>(args)...);
			return this->AddTimer(ms, methodProxy);
		}
		long long AddTimer(std::shared_ptr<TimerBase> timer);
		long long AddTimer(unsigned int ms, StaticMethod* func);

	 protected:
		bool Awake() final;

		bool LateAwake() final
		{
			return true;
		}

		void OnSystemUpdate();

		bool AddTimerToWheel(long long timerId);

		bool AddTimerToWheel(std::shared_ptr<TimerBase> timer);

	 private:
		const int LayerCount = 5;
		const int TimerPrecision = 20;
		const int OtherLayerCount = 32;
		const int FirstLayerCount = 256;
	 private:
		long long mNextUpdateTime;
		std::vector<TimeWheelLayer*> mTimerLayers;
		std::unordered_map<long long, std::shared_ptr<TimerBase>> mTimerMap;//所有timer的列表
	};
}// namespace Sentry