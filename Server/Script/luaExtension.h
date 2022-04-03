#pragma once

#include "Script/LuaInclude.h"

namespace Lua
{
	namespace LuaAPIExtension
	{
		extern int TypeCast(lua_State* luaEnv);

		extern int GameObjectGetComponent(lua_State* lua);

		extern int ComponentGetComponent(lua_State* lua);

		extern int GetComponent(lua_State* lua);

		extern int AddComponent(lua_State* lua);

		extern std::map<int, std::string> GetLuaStackData(lua_State* lua);
	}// namespace LuaAPIExtension
}