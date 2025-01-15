//
// Created by yjz on 2022/4/4.
//

#include"Timer.h"
#include"Entity/Actor/App.h"
#include<memory>
#include"Timer/Timer/DelayTimer.h"
#include"Timer/Component/TimerComponent.h"
using namespace acs;

namespace lua
{
	std::unique_ptr<LuaTimer> MakeTimer(lua_State * lua, int index, unsigned int ms)
	{
		int cor = 0;
		std::unique_ptr<LuaTimer> luaTimer = nullptr;
		if(lua_isfunction(lua, index))
		{
			lua_pushvalue(lua, index);
			int ref = luaL_ref(lua, LUA_REGISTRYINDEX);
			if(lua_pushthread(lua) == 0)
			{
				cor = luaL_ref(lua, LUA_REGISTRYINDEX);
			}
			luaTimer = std::make_unique<LuaTimer>(lua, ref, ms, cor);
		}
		else if(lua_istable(lua, index))
		{
			std::string method(luaL_checkstring(lua, index + 1));

			lua_pushvalue(lua, index);
			int ref = luaL_ref(lua, LUA_REGISTRYINDEX);
			if(lua_pushthread(lua) == 0)
			{
				cor = luaL_ref(lua, LUA_REGISTRYINDEX);
			}
			luaTimer = std::make_unique<LuaTimer>(lua, ref, method, ms, cor);
		}
		return luaTimer;
	}

	int Timer::Add(lua_State* lua)
    {
		static TimerComponent* timerComponent = nullptr;
		if(timerComponent == nullptr)
		{
			timerComponent = App::Get<TimerComponent>();
			if(timerComponent == nullptr)
			{
				luaL_error(lua, "not find TimerComponent");
				return 0;
			}
		}

		long long ms = luaL_checkinteger(lua, 1);
		std::unique_ptr<LuaTimer> luaTimer = MakeTimer(lua, 2, ms);
		if(luaTimer == nullptr)
		{
			lua_error(lua);
			return 0;
		}
        long long timerId = luaTimer->GetTimerId();
		timerComponent->AddTimer(std::move(luaTimer));
        lua_pushinteger(lua, timerId);
        return 1;
    }

	int Timer::AddUpdate(lua_State* lua)
	{
		static TimerComponent* timerComponent = nullptr;
		if(timerComponent == nullptr)
		{
			timerComponent = App::Get<TimerComponent>();
			if(timerComponent == nullptr)
			{
				luaL_error(lua, "not find TimerComponent");
				return 0;
			}
		}
		long long ms = luaL_checkinteger(lua, 1);
		std::unique_ptr<LuaTimer> luaTimer = MakeTimer(lua, 2, ms);
		if(luaTimer == nullptr)
		{
			lua_error(lua);
			return 0;
		}

		long long timerId = luaTimer->GetTimerId();
		timerComponent->AddUpdateTimer(std::move(luaTimer));
		lua_pushinteger(lua, timerId);
		return 1;
	}

	int Timer::Remove(lua_State* lua)
	{
		static TimerComponent* timerComponent = nullptr;
		if(timerComponent == nullptr)
		{
			timerComponent = App::Get<TimerComponent>();
			if(timerComponent == nullptr)
			{
				luaL_error(lua, "not find TimerComponent");
				return 0;
			}
		}
        long long timerId = luaL_checkinteger(lua, 1);//Lua::FromData::Read<unsigned int>(lua, 2);
        lua_pushboolean(lua, timerComponent->CancelTimer(timerId));
		return 1;
	}
}