#pragma once
#include<string>
#include"Lua/Engine/Define.h"
namespace Lua
{
	namespace Log
	{
		extern int Output(lua_State * lua);
		extern void Error(lua_State * lua);
		extern int OnCallError(lua_State * lua);
	}

	namespace Console
	{
		extern int Show(lua_State* luaEnv);
	}
}