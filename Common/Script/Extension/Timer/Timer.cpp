//
// Created by yjz on 2022/4/4.
//

#include"Timer.h"
#include"App/App.h"
#include<memory>
#include"Timer/DelayTimer.h"
#include"Script/UserDataParameter.h"
#include"Component/Timer/TimerComponent.h"
using namespace Sentry;

namespace Lua
{
	int Timer::AddTimer(lua_State* lua)
    {
        TimerComponent *timerComponent = Lua::UserDataParameter::Read<TimerComponent *>(lua, 1);
        unsigned int ms = Lua::Parameter::Read<unsigned int>(lua, 2);
        lua_pushvalue(lua, 3);
        if (!lua_isfunction(lua, -1))
        {
            luaL_error(lua, "add timer type = [%s]", luaL_typename(lua, -1));
            return 0;
        }
        int ref = luaL_ref(lua, LUA_REGISTRYINDEX);
        std::shared_ptr<LuaTimer> luaTimer(new LuaTimer(ms, ref, lua));
        unsigned int timerId = timerComponent->AddTimer(luaTimer);
        Lua::Parameter::Write<unsigned int>(lua, timerId);
        return 1;
    }

	int Timer::CancelTimer(lua_State* lua)
	{
        TimerComponent *timerComponent = Lua::UserDataParameter::Read<TimerComponent *>(lua, 1);
        unsigned int timerId = Lua::Parameter::Read<unsigned int>(lua, 2);
		Lua::Parameter::Write<bool>(lua, timerComponent->CancelTimer(timerId));
		return 1;
	}
}