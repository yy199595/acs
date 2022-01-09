#pragma once

#include <Script/LuaInclude.h>

namespace LuaAPIExtension
{
    extern int TypeCast(lua_State *luaEnv);

    extern int DebugLog(lua_State *luaEnv);

    extern int DebugInfo(lua_State *luaEnv);

    extern int DebugError(lua_State *luaEnv);

    extern int DebugWarning(lua_State *luaEnv);

    extern std::string GetLuaString(lua_State *luaEnv);

    extern int GameObjectGetComponent(lua_State *lua);

    extern int ComponentGetComponent(lua_State *lua);

    extern int GetComponent(lua_State *lua);

    extern int AddComponent(lua_State *lua);

    extern std::map<int, std::string> GetLuaStackData(lua_State *lua);
}// namespace LuaAPIExtension