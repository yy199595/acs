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
	inline SqliteComponent * GetComponent()
	{
		static SqliteComponent * sqliteComponent = nullptr;
		if(sqliteComponent == nullptr)
		{
			sqliteComponent = App::Get<SqliteComponent>();
			if(sqliteComponent == nullptr)
			{
				return nullptr;
			}
		}
		return sqliteComponent;
	}

	int Sqlite::Get(lua_State* L)
	{
		std::string value;
		std::string key(luaL_checkstring(L, 1));
		SqliteComponent * sqliteComponent = GetComponent();
		if(!sqliteComponent->Get(key, value))
		{
			return 0;
		}
		yyjson::write(L, value.c_str(), value.size());
		return 1;
	}

	int Sqlite::Set(lua_State* L)
	{
		std::string value;
		std::string key(luaL_checkstring(L, 1));
		luaL_checktype(L, 2, LUA_TTABLE);
		if(!yyjson::read(L, 2, value))
		{
			luaL_error(L, "cast json fail");
			return 0;
		}
		SqliteComponent * sqliteComponent = GetComponent();
		lua_pushboolean(L, sqliteComponent->Set(key, value));
		return 1;
	}

	int Sqlite::Del(lua_State* L)
	{
		std::string key(luaL_checkstring(L, 1));
		SqliteComponent * sqliteComponent = GetComponent();
		lua_pushboolean(L, sqliteComponent->Del(key));
		return 1;
	}

	int Sqlite::SetTimeout(lua_State* L)
	{
		std::string key(luaL_checkstring(L, 1));
		int timeout = (int)luaL_checkinteger(L, 2);
		SqliteComponent * sqliteComponent = GetComponent();
		lua_pushboolean(L, sqliteComponent->SetTimeout(key, timeout));
		return 1;
	}

	int Sqlite::Exec(lua_State* lua)
	{
		SqliteComponent * sqliteComponent = GetComponent();
		const char * sql = luaL_checkstring(lua, 1);
		lua_pushinteger(lua, sqliteComponent->Exec(sql));
		return 1;
	}

	int Sqlite::Query(lua_State* lua)
	{
		SqliteComponent * sqliteComponent = GetComponent();

		std::vector<std::string> result;
		const char * sql = luaL_checkstring(lua, 1);
		int code = sqliteComponent->Query(sql, result);
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
}
