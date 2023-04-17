#pragma once
#include<unordered_map>
#include"Timer/Timer/TimeWheelLayer.h"
#include"Entity/Component/Component.h"
#include"Rpc/Method/MethodProxy.h"
namespace Tendo
{
    class TimerComponent final : public Component, public ISystemUpdate, public ILuaRegister
	{
	 public:
		TimerComponent() = default;
	 public:
		bool CancelTimer(long long id);

		long long DelayCall(int ms, std::function<void(void)> && callback);

		template<typename F, typename O, typename ... Args>
		long long DelayCall(int ms, F&& f, O* o, Args&& ... args)
		{
			StaticMethod* methodProxy = NewMethodProxy(
				std::forward<F>(f), o, std::forward<Args>(args)...);
			return this->AddTimer(ms, methodProxy);
		}
		long long AddTimer(const std::shared_ptr<TimerBase>& timer);
		long long AddTimer(unsigned int ms, StaticMethod* func);

	 protected:
		bool Awake() final;

		bool LateAwake() final
		{
			return true;
		}

		void OnSystemUpdate() final;

		bool AddTimerToWheel(long long timerId);

		bool AddTimerToWheel(std::shared_ptr<TimerBase> timer);

        void OnLuaRegister(Lua::ClassProxyHelper &luaRegister) final;

    private:
		const int LayerCount = 5;
		const int TimerPrecision = 20;
		const int OtherLayerCount = 32;
		const int FirstLayerCount = 256;
	 private:
		long long mNextUpdateTime;
        std::queue<long long> mRemoveTimers;
		std::vector<TimeWheelLayer*> mTimerLayers;
		std::unordered_map<unsigned int, std::shared_ptr<TimerBase>> mTimerMap;//所有timer的列表
	};
}// namespace Sentry