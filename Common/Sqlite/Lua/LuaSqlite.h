//
// Created by yjz on 2023/3/24.
//

#ifndef APP_COMMON_SQLITE_LUA_LUASQLITE_H
#define APP_COMMON_SQLITE_LUA_LUASQLITE_H
#include"Lua/Engine/Define.h"
namespace Lua
{
	namespace Sqlite
	{
		int Open(lua_State* lua);
		int Exec(lua_State* lua);
		int Query(lua_State* lua);
		int QueryOnce(lua_State* lua);
	}
}
#endif //APP_COMMON_SQLITE_LUA_LUASQLITE_H
