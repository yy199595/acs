//
// Created by yjz on 2023/3/24.
//
#ifdef __ENABLE_MYSQL__


#ifndef APP_DATABASE_MYSQL_LUA_LUAMYSQL_H
#define APP_DATABASE_MYSQL_LUA_LUAMYSQL_H
#include"Lua/Engine/Define.h"
#include"Mysql/Client/MysqlMessage.h"
namespace Lua
{
	namespace LuaMysql
	{
		int Make(lua_State * lua);
		int Exec(lua_State * lua);
		int Query(lua_State * lua);
		int QueryOnce(lua_State * lua);
		int Invoke(lua_State * lua, int method);
	}
}

#endif //APP_DATABASE_MYSQL_LUA_LUAMYSQL_H

#endif