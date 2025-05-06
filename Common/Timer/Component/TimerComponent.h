#pragma once
#include<list>
#include<unordered_map>
#include"Rpc/Method/MethodProxy.h"
#include"Timer/Timer/TimeWheelLayer.h"
#include"Entity/Component/Component.h"

namespace acs
{
	class TimerComponent final : public Component, public ISystemUpdate, public IFrameUpdate, public IServerRecord
	{
	 public:
		TimerComponent();
	 public:
		bool CancelTimer(long long id);

		long long DelayCall(int ms, std::function<void(void)> && callback);

		template<typename F, typename O, typename ... Args>
		long long DelayCall(int ms, F&& f, O* o, Args&& ... args)
		{
			std::unique_ptr<StaticMethod> methodProxy = NewMethodProxy(
				std::forward<F>(f), o, std::forward<Args>(args)...);
			return this->CreateTimer(ms, std::move(methodProxy));
		}
	public:
		bool AddTimer(std::unique_ptr<TimerBase> timer);
		void AddUpdateTimer(std::unique_ptr<TimerBase> timer);
		long long CreateTimer(unsigned int ms, std::unique_ptr<StaticMethod> func);
	private:
		void OnNewDay();
		bool Awake() final;
		void OnSystemUpdate() noexcept final;
		void OnFrameUpdate(long long) noexcept final;
		void OnRecord(json::w::Document &document) final;
		bool InvokeTimer(long long timerId);
		bool AddTimerToWheel(long long timerId);
		bool AddTimerToWheel(std::unique_ptr<TimerBase> timer);
	 private:
		int mDoneCount;
		int mCancelCount;
		long long mNextUpdateTime;
		std::list<long long> mUpdateTimer;
		std::queue<long long> mLastFrameTriggerTimers;
		std::vector<std::unique_ptr<TimeWheelLayer>> mTimerLayers;
		std::unordered_map<long long, std::unique_ptr<TimerBase>> mTimerMap;//所有timer的列表
	};
}// namespace Sentry