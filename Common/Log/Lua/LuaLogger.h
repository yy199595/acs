#pragma once
#include<string>
#include"Lua/Engine/Define.h"
namespace Lua
{
	namespace Log
	{
		extern int Debug(lua_State* luaEnv);

		extern int Info(lua_State* luaEnv);

		extern int Error(lua_State* luaEnv);

		extern int Warning(lua_State* luaEnv);

		extern int LuaError(lua_State* luaEnv);

		extern void GetLuaString(lua_State* luaEnv, std::string & ret);
	}

	namespace Console
	{
		extern int Debug(lua_State* luaEnv);

		extern int Info(lua_State* luaEnv);

		extern int Error(lua_State* luaEnv);

		extern int Warning(lua_State* luaEnv);
	}
}