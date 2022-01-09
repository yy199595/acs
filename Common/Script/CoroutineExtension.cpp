#include "CoroutineExtension.h"
#include <Core/App.h>
#include"Timer/LuaSleepTimer.h"
using namespace GameKeeper;
namespace CoroutineExtension
{
    int Sleep(lua_State *lua)
    {
        lua_pushthread(lua);
        long long ms = lua_tointeger(lua, 1);
        auto timer = LuaSleepTimer::Create(lua, -1, ms);
        auto timerComponent = App::Get().GetTimerComponent();
        lua_pushinteger(lua, timerComponent->AddTimer(timer));
        return lua_yield(lua, 0);
    }

    int Start(lua_State *lua)
    {
        LOG_CHECK_RET_ZERO(lua_isfunction(lua, 1));
        lua_State *coroutine = lua_newthread(lua);
        lua_pushvalue(lua, 1);
        lua_xmove(lua, coroutine, 1);
        lua_replace(lua, 1);

        const int size = lua_gettop(lua);
        lua_xmove(lua, coroutine, size - 1);
		lua_presume(coroutine, lua, size - 1);

        return 1;
    }
}// namespace CoroutineExtension
