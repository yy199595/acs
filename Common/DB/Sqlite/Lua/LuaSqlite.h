//
// Created by yjz on 2023/3/24.
//

#ifndef APP_COMMON_SQLITE_LUA_LUASQLITE_H
#define APP_COMMON_SQLITE_LUA_LUASQLITE_H
#include"Lua/Engine/Define.h"
namespace lua
{
	namespace Sqlite
	{
		int Set(lua_State * lua);
		int Get(lua_State * lua);
		int Del(lua_State * lua);
		int SetTimeout(lua_State * lua);

		int Run(lua_State* lua);
		int Build(lua_State * lua);
		int Invoke(lua_State * lua);
	}
}
#endif //APP_COMMON_SQLITE_LUA_LUASQLITE_H
