//
// Created by yjz on 2022/5/15.
//

#ifndef _LUAAPP_H_
#define _LUAAPP_H_
#include"Lua/LuaInclude.h"
namespace Lua
{
	namespace LuaApp
	{
		extern int Send(lua_State * lua);
		extern int Call(lua_State * lua);
		extern int GetComponent(lua_State * lua);
	};
}

#endif //_LUAAPP_H_
