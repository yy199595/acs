//
// Created by 64658 on 2023/3/19.
//

#ifndef APP_LUAGATE_H
#define APP_LUAGATE_H
#include"Lua/LuaInclude.h"
namespace Lua
{
    namespace Gate
    {
        extern int Send(lua_State * lua);
        extern int BroadCast(lua_State * lua);
    };
}


#endif //APP_LUAGATE_H
