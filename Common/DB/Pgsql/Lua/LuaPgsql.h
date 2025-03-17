//
// Created by yjz on 2023/3/24.
//


#ifndef APP_DATABASE_PGSQL_LUA_LUAMYSQL_H
#define APP_DATABASE_PGSQL_LUA_LUAMYSQL_H
#include"Lua/Engine/LuaInclude.h"
namespace lua
{
	namespace lpgsql
	{
		int Run(lua_State * lua);
	}
}

#endif //APP_DATABASE_PGSQL_LUA_LUAMYSQL_H
