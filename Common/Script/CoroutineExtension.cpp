#include "CoroutineExtension.h"
#include <Core/App.h>

using namespace GameKeeper;
namespace CoroutineExtension
{
    int Sleep(lua_State *lua)
    {
        /*const long long ms = lua_tointeger(lua, -1);
        GKAssertRetZero_F(lua_getfunction(lua, "coroutine", "running"));
        if (lua_pcall(lua, 0, 1, 0) != 0)
        {
            GKDebugError(lua_tostring(lua, -1));
            return 0;
        }
        GKAssertRetZero_F(lua_isthread(lua, -1));
        int ref = luaL_ref(lua, LUA_REGISTRYINDEX);
        App * frameWork = App::Get();
        GKTimerManager * timerManager = frameWork->GetTimerManager();
        GKLuaSleepTimer * timer = Object::CreateObject<GKLuaSleepTimer>(lua, ref, ms);
        timerManager->AddTimer(timer);
        return lua_yield(lua, 01);*/
        return 0;
    }

    int Start(lua_State *lua)
    {
        GKAssertRetZero_F(lua_isfunction(lua, 1));
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
