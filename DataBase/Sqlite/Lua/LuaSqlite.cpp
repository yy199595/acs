//
// Created by yjz on 2023/3/24.
//

#include "LuaSqlite.h"
#include"Entity/App/App.h"
#include"Util/Json/Lua/Json.h"
#include"Sqlite/Component/SqliteComponent.h"
using namespace Sentry;
namespace Lua
{
	int Sqlite::Open(lua_State* lua)
	{
		SqliteComponent * sqliteComponent = App::Inst()->GetComponent<SqliteComponent>();
		if(sqliteComponent == nullptr)
		{
			luaL_error(lua, "not find SqliteComponent");
			return 0;
		}
		const char * name = luaL_checkstring(lua, 1);
		int id = sqliteComponent->Open(name);
		lua_pushinteger(lua, id);
		return 1;
	}

	int Sqlite::Exec(lua_State* lua)
	{
		SqliteComponent * sqliteComponent = App::Inst()->GetComponent<SqliteComponent>();
		if(sqliteComponent == nullptr)
		{
			luaL_error(lua, "not find SqliteComponent");
			return 0;
		}
		int id = (int)luaL_checkinteger(lua, 1);
		const char * sql = luaL_checkstring(lua, 2);
		bool res = sqliteComponent->Exec(id, sql);
		lua_pushboolean(lua, res);
		return 1;
	}

	int Sqlite::Query(lua_State* lua)
	{
		SqliteComponent * sqliteComponent = App::Inst()->GetComponent<SqliteComponent>();
		if(sqliteComponent == nullptr)
		{
			luaL_error(lua, "not find SqliteComponent");
			return 0;
		}
		std::vector<std::string> result;
		int id = (int)luaL_checkinteger(lua, 1);
		const char * sql = luaL_checkstring(lua, 2);
		if(!sqliteComponent->Query(id, sql, result) || result.empty())
		{
			return 0;
		}
		int index = 0;
		int size = (int)result.size();
		lua_createtable(lua, 0, size);
		for(const std::string & json : result)
		{
			lua_pushinteger(lua, ++index);
			Lua::RapidJson::Write(lua, json);
			lua_settable(lua, -3);
		}
		return 1;
	}

	int Sqlite::QueryOnce(lua_State* lua)
	{
		SqliteComponent * sqliteComponent = App::Inst()->GetComponent<SqliteComponent>();
		if(sqliteComponent == nullptr)
		{
			luaL_error(lua, "not find SqliteComponent");
			return 0;
		}
		size_t size = 0;
		std::vector<std::string> result;
		int id = (int)luaL_checkinteger(lua, 1);
		const char * sql = luaL_checklstring(lua, 2, &size);
		if(!sqliteComponent->Query(id, sql, result) || result.empty())
		{
			return 0;
		}
		Lua::RapidJson::Write(lua, result.at(0));
		return 1;
	}
}
