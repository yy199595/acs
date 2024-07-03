//
// Created by yjz on 2023/3/24.
//
#ifdef __ENABLE_MYSQL__


#include"LuaMysql.h"
#include"Entity/Actor/App.h"
#include"Mysql/Lua/LuaMysqlTask.h"
#include"Mysql/Client/MysqlMessage.h"
#include"Proto/Include/Message.h"
#include"Mysql/Component/MysqlDBComponent.h"
#include"Proto/Component/ProtoComponent.h"
using namespace joke;
namespace Lua
{
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
		static MysqlDBComponent* mysqlComponent = nullptr;
		if(mysqlComponent == nullptr)
		{
			mysqlComponent = App::Inst()->GetComponent<MysqlDBComponent>();
			if(mysqlComponent == nullptr)
			{
				luaL_error(lua, "not find [MysqlDBComponent]");
				return 0;
			}
		}
		int rpcId = 0;
		size_t size = 0;
		std::unique_ptr<Mysql::IRequest> command;
		switch (method)
		{
			case LUA_MYSQL_EXEC:
			{
				const char* sql = luaL_checklstring(lua, 1, &size);
				command = std::make_unique<Mysql::SqlRequest>(std::string(sql, size));
			}
				break;
			case LUA_MYSQL_QUERY:
			case LUA_MYSQL_QUERY_ONE:
			{
				const char* sql = luaL_checklstring(lua, 1, &size);
				command = std::make_unique<Mysql::FindRequest>(std::string(sql, size));
			}
				break;
			case LUA_MYSQL_CREATE_TABLE:
			{
				std::unique_ptr<pb::Message> message;
				const char* tb = luaL_checkstring(lua, 1);
				const char* pb = luaL_checkstring(lua, 2);
				ProtoComponent* messageComponent = App::Inst()->GetProto();
				if (lua_istable(lua, 3))
				{
					pb::Message * temp = messageComponent->Read(lua, pb, 4);
					if(temp == nullptr)
					{
						luaL_error(lua, "create %s error", pb);
						return 0;
					}
					message.reset(temp->New());
				}
				else
				{
					if (!messageComponent->New(pb, message))
					{
						luaL_error(lua, "create %s error", pb);
						return 0;
					}
				}
				std::vector<std::string> keys;
				if (lua_istable(lua, 4))
				{
					lua_pushnil(lua);
					while (lua_next(lua, -2) != 0)
					{
						const char* key = luaL_checkstring(lua, -1);
						keys.emplace_back(key);
						lua_pop(lua, 1);
					}
				}
				command = std::make_unique<Mysql::MakeTabRequest>(tb, std::move(message), keys);
			}
				break;
			default:
				return 0;
		}
		lua_pushthread(lua);
		if (!mysqlComponent->Send(std::move(command), rpcId))
		{
			luaL_error(lua, "send mysql command error");
			return 0;
		}
		return mysqlComponent->AddTask(rpcId, new LuaMysqlTask(lua, rpcId))->Await();
	}
}

#endif