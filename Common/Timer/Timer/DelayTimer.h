#pragma once

#include"TimerBase.h"
#include"Lua/Engine/Define.h"
#include"Rpc/Method/MethodProxy.h"
namespace joke
{
	class DelayTimer : public TimerBase
	{
	public:
		DelayTimer(long long ms, StaticMethod * func)
			: TimerBase(ms), mFunc(func) {}
        ~DelayTimer() final { delete this->mFunc;}
	public:
		void Invoke() final { this->mFunc->run(); }
	private:
		StaticMethod * mFunc;
	};
}// namespace Sentry

namespace joke
{
	class LuaTimer final : public TimerBase
	{
	public:
		LuaTimer(lua_State * lua, int ref, long long ms)
			: TimerBase(ms), ref(ref), mLua(lua) { }
		LuaTimer(lua_State * lua, int ref, std::string func, long long ms)
				: TimerBase(ms), ref(ref), mMethod(std::move(func)), mLua(lua) { }
		~LuaTimer() override { luaL_unref(this->mLua, LUA_REGISTRYINDEX, ref);}
	public:
		void Invoke() final;
	private:
		int ref;
		lua_State * mLua;
		std::string mMethod;
	};
}