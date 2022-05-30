//
// Created by mac on 2022/5/30.
//

#ifndef SERVER_LUAHTTP_H
#define SERVER_LUAHTTP_H
#include"Script/LuaInclude.h"
namespace Lua
{
	namespace Http
	{
		extern int Get(lua_State* lua);
		extern int Post(lua_State* lua);
	}
}


#endif //SERVER_LUAHTTP_H
