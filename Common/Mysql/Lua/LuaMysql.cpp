//
// Created by yjz on 2023/3/24.
//
#ifdef __ENABLE_MYSQL__


#include"LuaMysql.h"
#include"Entity/Unit/App.h"
#include"Mysql/Lua/LuaMysqlTask.h"
#include"Mysql/Client/MysqlMessage.h"
#include"Mysql/Component/MysqlDBComponent.h"
#include"Proto/Component/ProtoComponent.h"
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

	int LuaMysql::CreateTable(lua_State* lua)
	{
		return Invoke(lua, LUA_MYSQL_CREATE_TABLE);
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
		int id = (int)luaL_checkinteger(lua, 1);
		std::shared_ptr<Mysql::ICommand> command;
		switch (method)
		{
			case LUA_MYSQL_EXEC:
			{
				const char* sql = luaL_checklstring(lua, 2, &size);
				command = std::make_shared<Mysql::SqlCommand>(std::string(sql, size));
			}
				break;
			case LUA_MYSQL_QUERY:
			case LUA_MYSQL_QUERY_ONE:
			{
				const char* sql = luaL_checklstring(lua, 2, &size);
				command = std::make_shared<Mysql::QueryCommand>(std::string(sql, size));
			}
				break;
			case LUA_MYSQL_CREATE_TABLE:
			{
				std::shared_ptr<Message> message;
				const char * tb = luaL_checkstring(lua, 2);
				const char * pb = luaL_checkstring(lua, 3);
				ProtoComponent* messageComponent = App::Inst()->GetProto();
				if(lua_istable(lua, 4))
				{
					message = messageComponent->Read(lua, pb, 4);
					if (message == nullptr)
					{
						luaL_error(lua, "create %s error", pb);
						return 0;
					}
				}
				else
				{
					if(!messageComponent->New(pb, message))
					{
						return 0;
					}
				}
				std::vector<std::string> keys;
				if(lua_istable(lua, 5))
				{
					lua_pushnil(lua);
					while (lua_next(lua, -2) != 0)
					{
						const char * key = luaL_checkstring(lua, -1);
						keys.emplace_back(key);
						lua_pop(lua, 1);
					}
				}
				command = std::make_shared<Mysql::CreateTabCommand>(tb, message, keys);
			}
				break;
			default:
				return 0;
		}
		if (id == 0)
		{
			std::shared_ptr<Mysql::Response> response = component->Run(command);
			return response->WriteToLua(lua);
		}
		lua_pushthread(lua);
		if (!component->Send(id, command, rpcId))
		{
			luaL_error(lua, "send mysql command error");
			return 0;
		}
		std::shared_ptr<LuaMysqlTask> luaMysqlTask
			= std::make_shared<LuaMysqlTask>(lua, rpcId);
		return component->AddTask(rpcId, luaMysqlTask)->Await();
	}
}

#endif