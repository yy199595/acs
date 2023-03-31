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

namespace Sentry
{

	class LuaMysqlTask : public IRpcTask<Mysql::Response>
	{
	public:
		LuaMysqlTask(lua_State* lua, int id, int method);
		~LuaMysqlTask();
	public:
		int Await();
		void OnResponse(std::shared_ptr<Mysql::Response> response) final;
	private:
		int mRef;
		int mMethod;
		lua_State* mLua;
	};
}


#endif //APP_DATABASE_MYSQL_LUA_LUAMYSQLTASK_H

#endif