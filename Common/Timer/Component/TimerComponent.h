#pragma once
#include<list>
#include<unordered_map>
#include"Rpc/Method/MethodProxy.h"
#include"Timer/Timer/TimeWheelLayer.h"
#include"Entity/Component/Component.h"
namespace acs
{
    class TimerComponent final : public Component, public ISystemUpdate,
								 public ILuaRegister, public IFrameUpdate
	{
	 public:
		TimerComponent();
	 public:
		bool CancelTimer(long long id);

		long long DelayCall(int ms, std::function<void(void)> && callback);

		template<typename F, typename O, typename ... Args>
		long long DelayCall(int ms, F&& f, O* o, Args&& ... args)
		{
			StaticMethod* methodProxy = NewMethodProxy(
				std::forward<F>(f), o, std::forward<Args>(args)...);
			return this->CreateTimer(ms, methodProxy);
		}
	public:
		bool AddTimer(std::unique_ptr<TimerBase> timer);
		void AddUpdateTimer(std::unique_ptr<TimerBase> timer);
		long long CreateTimer(unsigned int ms, StaticMethod* func);
	private:
		bool Awake() final;
		void OnSystemUpdate() noexcept final;
		void OnFrameUpdate(long long) noexcept final;
		bool InvokeTimer(long long timerId);
		bool AddTimerToWheel(long long timerId);
		bool AddTimerToWheel(std::unique_ptr<TimerBase> timer);
      	void OnLuaRegister(Lua::ModuleClass &luaRegister) final;
    private:
		const int LayerCount = 5;
		const int TimerPrecision = 20;
		const int OtherLayerCount = 32;
		const int FirstLayerCount = 256;
	 private:
		long long mNextUpdateTime;
		std::list<long long> mUpdateTimer;
		std::vector<TimeWheelLayer*> mTimerLayers;
		std::queue<long long> mLastFrameTriggerTimers;
		std::unordered_map<long long, std::unique_ptr<TimerBase>> mTimerMap;//所有timer的列表
	};
}// namespace Sentry