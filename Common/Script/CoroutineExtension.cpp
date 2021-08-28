#include "CoroutineExtension.h"
#include <Core/App.h>

using namespace Sentry;
namespace CoroutineExtension
{
    int Sleep(lua_State *lua)
    {
        /*const long long ms = lua_tointeger(lua, -1);
        SayNoAssertRetZero_F(lua_getfunction(lua, "coroutine", "running"));
        if (lua_pcall(lua, 0, 1, 0) != 0)
        {
            SayNoDebugError(lua_tostring(lua, -1));
            return 0;
        }
        SayNoAssertRetZero_F(lua_isthread(lua, -1));
        int ref = luaL_ref(lua, LUA_REGISTRYINDEX);
        App * frameWork = App::Get();
        SayNoTimerManager * timerManager = frameWork->GetTimerManager();
        SayNoLuaSleepTimer * timer = Object::CreateObject<SayNoLuaSleepTimer>(lua, ref, ms);
        timerManager->AddTimer(timer);
        return lua_yield(lua, 01);*/
        return 0;
    }

    int Start(lua_State *lua)
    {
        SayNoAssertRetZero_F(lua_isfunction(lua, 1));
        lua_State *coroutine = lua_newthread(lua);
        lua_pushvalue(lua, 1);
        lua_xmove(lua, coroutine, 1);
        lua_replace(lua, 1);

        const int size = lua_gettop(lua);
        lua_xmove(lua, coroutine, size - 1);
        lua_resume(coroutine, lua, size - 1);

        return 1;
    }
}// namespace CoroutineExtension
