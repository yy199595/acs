#pragma once

#include <Script/LuaInclude.h>

namespace SystemExtension
{
    extern int AddTimer(lua_State *lua);

    extern int RemoveTimer(lua_State *lua);

    extern int Sleep(lua_State *luaEnv);

    extern int GetApp(lua_State *luaEnv);

    extern int Call(lua_State * luaEnv);

    extern int Allot(lua_State * luaEnv);

    extern int GetManager(lua_State *luaEnv);

    extern bool RequireLua(lua_State *luaEnv, const char *name);
};// namespace SystemExtension
