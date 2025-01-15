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
		int Exec(lua_State* lua);
		int Find(lua_State* lua);
		int FindOne(lua_State* lua);
	}
}
#endif //APP_COMMON_SQLITE_LUA_LUASQLITE_H
