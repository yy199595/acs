//
// Created by yjz on 2023/3/24.
//
#ifdef __ENABLE_MYSQL__

#ifndef APP_DATABASE_MYSQL_LUA_LUAMYSQLTASK_H
#define APP_DATABASE_MYSQL_LUA_LUAMYSQLTASK_H
#include"Mysql/Client/MysqlMessage.h"
#include"Rpc/Async/RpcTaskSource.h"
struct lua_State;

constexpr int LUA_MYSQL_EXEC = 1;
constexpr int LUA_MYSQL_QUERY = 2;
constexpr int LUA_MYSQL_QUERY_ONE = 3;
constexpr int LUA_MYSQL_CREATE_TABLE = 4;

namespace joke
{

	class LuaMysqlTask : public IRpcTask<Mysql::Response>
	{
	public:
		LuaMysqlTask(lua_State* lua, int id);
		~LuaMysqlTask();
	public:
		int Await();
		void OnResponse(Mysql::Response * response) final;
	private:
		int mRef;
		lua_State* mLua;
	};
}


#endif //APP_DATABASE_MYSQL_LUA_LUAMYSQLTASK_H

#endif