//
// Created by yjz on 2023/3/24.
//

#include "LuaSqlite.h"
#include"XCode/XCode.h"
#include"Entity/Actor/App.h"
#include"Yyjson/Lua/ljson.h"
#include"Sqlite/Component/SqliteComponent.h"
using namespace acs;
namespace lua
{
	int Sqlite::Exec(lua_State* lua)
	{
		static SqliteComponent * sqliteComponent = nullptr;
		if(sqliteComponent == nullptr)
		{
			sqliteComponent = App::Get<SqliteComponent>();
			if(sqliteComponent == nullptr)
			{
				luaL_error(lua, "not find SqliteComponent");
				return 0;
			}
		}
		std::string db(luaL_checkstring(lua, 1));
		const char * sql = luaL_checkstring(lua, 2);
		lua_pushinteger(lua, sqliteComponent->Exec(db, sql));
		return 1;
	}

	int Sqlite::Find(lua_State* lua)
	{
		static SqliteComponent * sqliteComponent = nullptr;
		if(sqliteComponent == nullptr)
		{
			sqliteComponent = App::Get<SqliteComponent>();
			if(sqliteComponent == nullptr)
			{
				luaL_error(lua, "not find SqliteComponent");
				return 0;
			}
		}
		std::vector<std::string> result;
		std::string db(luaL_checkstring(lua, 1));
		const char * sql = luaL_checkstring(lua, 2);
		int code = sqliteComponent->Query(db, sql, result);
		if(code != XCode::Ok)
		{
			lua_pushinteger(lua, code);
			return 1;
		}
		int index = 0;
		int size = (int)result.size();
		lua_pushinteger(lua, code);
		lua_createtable(lua, 0, size);
		for(const std::string & json : result)
		{
			lua_pushinteger(lua, ++index);
			lua::yyjson::write(lua, json.c_str(), json.size());
			lua_settable(lua, -3);
		}
		return 2;
	}

	int Sqlite::FindOne(lua_State* lua)
	{
		static SqliteComponent * sqliteComponent = nullptr;
		if(sqliteComponent == nullptr)
		{
			sqliteComponent = App::Get<SqliteComponent>();
			if(sqliteComponent == nullptr)
			{
				luaL_error(lua, "not find SqliteComponent");
				return 0;
			}
		}
		size_t size = 0;
		std::vector<std::string> result;
		std::string db(lua_tostring(lua, 1));
		const char * sql = luaL_checklstring(lua, 2, &size);
		int code = sqliteComponent->Query(db, sql, result);
		if(code != XCode::Ok)
		{
			lua_pushinteger(lua, code);
			return 1;
		}
		lua_pushinteger(lua, code);
		const std::string & json = result.at(0);
		lua::yyjson::write(lua, json.c_str(), json.size());
		return 2;
	}
}
