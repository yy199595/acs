//
// Created by zmhy0073 on 2022/1/10.
//

#ifndef GAMEKEEPER_LUAHELPER_H
#define GAMEKEEPER_LUAHELPER_H
#include"Script/LuaInclude.h"
namespace Helper
{
    namespace Lua
    {
        template<typename T>
        bool IsType(lua_State *luaEnv, int index)
        { return false; }

        template<int>
        bool IsType(lua_State *lua, int index)
        { return lua_isinteger(lua, index); }

        template<short>
        bool IsType(lua_State *lua, int index)
        { return lua_isinteger(lua, index); }

        template<unsigned short>
        bool IsType(lua_State *lua, int index)
        { return lua_isinteger(lua, index); }

        template<unsigned int>
        bool IsType(lua_State *lua, int index)
        { return lua_isinteger(lua, index); }

        template<long long>
        bool IsType(lua_State *lua, int index)
        { return lua_isinteger(lua, index); }

        template<unsigned long>
        bool IsType(lua_State *lua, int index)
        { return lua_isinteger(lua, index); }

    }
}
#endif //GAMEKEEPER_LUAHELPER_H
