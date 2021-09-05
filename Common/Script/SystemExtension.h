#pragma once

#include <Script/LuaInclude.h>

namespace SystemExtension
{
    extern int Call(lua_State *luaEnv);

    extern int CallWait(lua_State *luaEnv);

    extern int CallByName(lua_State *luaEnv);

    extern int CallBySession(lua_State *luaEnv);

    extern int AddTimer(lua_State *lua);

    extern int RemoveTimer(lua_State *lua);

    extern int Sleep(lua_State *luaEnv);

    extern int GetApp(lua_State *luaEnv);

    extern int GetManager(lua_State *luaEnv);

    extern int LuaRetMessage(lua_State *luaEnv);

    extern bool RequireLua(lua_State *luaEnv, const char *name);
};// namespace SystemExtension
