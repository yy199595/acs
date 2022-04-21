//
// Created by yjz on 2022/4/4.
//

#include"Timer.h"
#include"App/App.h"
#include<memory>
#include"Script/Function.h"
#include"Component/Timer/TimerComponent.h"
using namespace Sentry;
namespace Lua
{
	int Timer::AddTimer(lua_State* lua)
	{
//		int ms = Lua::Parameter::Read<int>(lua, 1);
//		lua_pushvalue(lua, 2);
//		if(!lua_isfunction(lua, -1))
//		{
//			luaL_error(lua, "add timer type = [%s]", luaL_typename(lua, -1));
//			return 0;
//		}
//		int ref = luaL_ref(lua, LUA_REGISTRYINDEX);
//		std::function<void(void)> callback = [lua, ref]()
//		{
//			lua_rawgeti(lua, LUA_REGISTRYINDEX, ref);
//			if (lua_pcall(lua, 0, 0, 0) != 0)
//			{
//				luaL_error(lua, "[call function ] = %s", lua_tostring(lua, -1));
//			}
//			luaL_unref(lua, LUA_REGISTRYINDEX, ref);
//		};
//		TimerComponent* timerComponent = App::Get()->GetComponent<TimerComponent>();
//		long long timerId = timerComponent->AddTimer(ms, new LambdaMethod(std::move(callback)));
//		Lua::Parameter::Write<long long>(lua, timerId);
		return 1;
	}

	int Timer::CancelTimer(lua_State* lua)
	{
		long long timerId = Lua::Parameter::Read<long long>(lua, 1);
		TimerComponent * timerComponent = App::Get()->GetComponent<TimerComponent>();
		Lua::Parameter::Write<bool>(lua, timerComponent->CancelTimer(timerId));
		return 1;
	}
}