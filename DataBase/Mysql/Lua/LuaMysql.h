//
// Created by yjz on 2023/3/24.
//

#ifndef APP_DATABASE_MYSQL_LUA_LUAMYSQL_H
#define APP_DATABASE_MYSQL_LUA_LUAMYSQL_H
#include"Lua/LuaInclude.h"
#include"Client/MysqlMessage.h"
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
