#pragma once

#include"TimerBase.h"
#include"Script/Lua/LuaInclude.h"
#include"Rpc/Method/MethodProxy.h"
namespace Tendo
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

namespace Tendo
{
	class LuaTimer final : public TimerBase
	{
	public:
		LuaTimer(long long ms, int ref, lua_State * lua)
			: TimerBase(ms), ref(ref), mLua(lua) { }
			~LuaTimer() { luaL_unref(this->mLua, LUA_REGISTRYINDEX, ref);}
	public:
		void Invoke() final
		{
			lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, ref);
			if (lua_pcall(this->mLua, 0, 0, 0) != 0)
			{
				luaL_error(this->mLua, "[call function ] = %s", lua_tostring(this->mLua, -1));
			}
		}
	private:
		int ref;
		lua_State * mLua;
	};
}