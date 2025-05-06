#pragma once

#include"TimerBase.h"
#include"Lua/Engine/Define.h"
#include"Rpc/Method/MethodProxy.h"
namespace acs
{
	class DelayTimer final : public TimerBase
	{
	public:
		DelayTimer(long long ms, std::unique_ptr<StaticMethod> func)
			: TimerBase(ms), mFunc(std::move(func)) {}
	public:
		void Invoke() final { this->mFunc->run(); }
	private:
		std::unique_ptr<StaticMethod> mFunc;
	};
}// namespace Sentry

namespace acs
{
	class LuaTimer final : public TimerBase
	{
	public:
		LuaTimer(lua_State * lua, int ref, long long ms, int cor)
			: TimerBase(ms), ref(ref), cor(cor), mLua(lua) { }
		LuaTimer(lua_State * lua, int ref, std::string func, long long ms, int cor)
				: TimerBase(ms), ref(ref), cor(cor), mMethod(std::move(func)), mLua(lua) { }
		~LuaTimer() override;
	public:
		void Invoke() final;
	private:
		int ref;
		int cor;
		lua_State * mLua;
		std::string mMethod;
	};
}