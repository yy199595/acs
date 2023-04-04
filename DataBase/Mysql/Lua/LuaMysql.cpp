//
// Created by yjz on 2023/3/24.
//
#ifdef __ENABLE_MYSQL__


#include"LuaMysql.h"
#include"Entity/App/App.h"
#include"Mysql/Lua/LuaMysqlTask.h"
#include"Mysql/Client/MysqlMessage.h"
#include"Mysql/Component/MysqlDBComponent.h"
using namespace Tendo;
namespace Lua
{
	int LuaMysql::Make(lua_State* lua)
	{
		MysqlDBComponent* component =
			App::Inst()->GetComponent<MysqlDBComponent>();
		if (component == nullptr)
		{
			luaL_error(lua, "not find [MysqlDBComponent]");
			return 0;
		}
		int id = 0;
		if (!component->GetClientHandle(id))
		{
			luaL_error(lua, "get mysql client handle error");
			return 0;
		}
		lua_pushinteger(lua, id);
		return 1;
	}

	int LuaMysql::Exec(lua_State* lua)
	{
		return Invoke(lua, LUA_MYSQL_EXEC);
	}

	int LuaMysql::Query(lua_State* lua)
	{
		return Invoke(lua, LUA_MYSQL_QUERY);
	}

	int LuaMysql::QueryOnce(lua_State* lua)
	{
		return Invoke(lua, LUA_MYSQL_QUERY_ONE);
	}

	int LuaMysql::Invoke(lua_State* lua, int method)
	{
		MysqlDBComponent* component =
			App::Inst()->GetComponent<MysqlDBComponent>();
		if (component == nullptr)
		{
			luaL_error(lua, "not find [MysqlDBComponent]");
			return 0;
		}
		int rpcId = 0;
		size_t size = 0;
		lua_pushthread(lua);
		int id = (int)luaL_checkinteger(lua, 1);
		const char* sql = luaL_checklstring(lua, 2, &size);
		std::shared_ptr<Mysql::ICommand> command;
		switch (method)
		{
			case LUA_MYSQL_EXEC:
				command = std::make_shared<Mysql::SqlCommand>(std::string(sql, size));
				break;
			case LUA_MYSQL_QUERY:
			case LUA_MYSQL_QUERY_ONE:
				command = std::make_shared<Mysql::QueryCommand>(std::string(sql, size));
				break;
			default:
				return 0;
		}
		if (!component->Send(id, command, rpcId))
		{
			luaL_error(lua, "send mysql command error");
			return 0;
		}
		std::shared_ptr<LuaMysqlTask> luaMysqlTask
			= std::make_shared<LuaMysqlTask>(lua, rpcId, method);
		return component->AddTask(rpcId, luaMysqlTask)->Await();
	}
}

#endif