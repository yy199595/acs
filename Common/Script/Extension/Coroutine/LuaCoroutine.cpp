#include"LuaCoroutine.h"
#include"App/App.h"
#include"Define/CommonLogDef.h"
#include"Component/Timer/TimerComponent.h"
using namespace Sentry;
namespace Sentry
{
	LuaSleepTimer::LuaSleepTimer(lua_State *lua, int ref, long long ms)
		: TimerBase(ms)
	{
		this->mRef = ref;
		this->mLuaEnv = lua;
	}

	std::shared_ptr<LuaSleepTimer> LuaSleepTimer::Create(lua_State *lua, int index, long long ms)
	{
		if (!lua_isthread(lua, index))
		{
			return nullptr;
		}
		int ref = luaL_ref(lua, LUA_REGISTRYINDEX);
		return std::make_shared<LuaSleepTimer>(lua, ref, ms);
	}

	void LuaSleepTimer::Invoke(TimerState state)
	{
		lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->mRef);
		if (!lua_isthread(this->mLuaEnv, -1))
		{
			LOG_ERROR("invoke lua sleep timer error");
			return;
		}
		lua_State *co = lua_tothread(this->mLuaEnv, -1);
		lua_presume(co, this->mLuaEnv, 0);
	}
}// namespace Sentry

namespace Lua
{
	int Coroutine::Sleep(lua_State* lua)
	{
		lua_pushthread(lua);
		long long ms = lua_tointeger(lua, 1);
		auto timer = LuaSleepTimer::Create(lua, -1, ms);
		TimerComponent * timerComponent = App::Get()->GetTimerComponent();
		lua_pushinteger(lua, timerComponent->AddTimer(timer));
		return lua_yield(lua, 0);
	}

	int Coroutine::Start(lua_State* lua)
	{
		if(!lua_isfunction(lua, 1))
		{
			luaL_error(lua, "type error");
			return 0;
		}
		lua_State* coroutine = lua_newthread(lua);
		lua_pushvalue(lua, 1);
		lua_xmove(lua, coroutine, 1);
		lua_replace(lua, 1);

		const int size = lua_gettop(lua);
		lua_xmove(lua, coroutine, size - 1);
		lua_presume(coroutine, lua, size - 1);
		return 1;
	}
}
