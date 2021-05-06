#include"LuaSleepTimer.h"

namespace SoEasy
{
	LuaSleepTimer::LuaSleepTimer(lua_State * lua, int ref, long long ms)
		: TimerBase(ms)
	{
		this->mRef = ref;
		this->mLuaEnv = lua;
	}

	shared_ptr<LuaSleepTimer> LuaSleepTimer::Create(lua_State * lua, int index, long long ms)
	{
		if (!lua_isthread(lua, index))
		{
			return nullptr;
		}
		int ref = luaL_ref(lua, LUA_REGISTRYINDEX);
		return std::make_shared<LuaSleepTimer>(lua, ref, ms);
	}

	bool LuaSleepTimer::Invoke()
	{
		lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->mRef);
		if (!lua_isthread(this->mLuaEnv, -1))
		{
			return true;
		}
		lua_State * coroutine = lua_tothread(this->mLuaEnv, -1);
		lua_resume(coroutine, this->mLuaEnv, 0);
		return true;
	}
}
