//
// Created by yjz on 2023/3/24.
//



#include"LuaPgsqlTask.h"
#include"Lua/Engine/Define.h"
#include "Yyjson/Lua/ljson.h"
namespace acs
{
	LuaPgsqlTask::LuaPgsqlTask(lua_State *lua, int id)
		: IRpcTask<pgsql::Response>(id), mLua(lua)
	{
		this->mRef = 0;
		if(lua_isthread(this->mLua, -1))
		{
			this->mRef = luaL_ref(lua, LUA_REGISTRYINDEX);
		}
	}
	LuaPgsqlTask::~LuaPgsqlTask()
	{
		if(this->mRef != 0)
		{
			luaL_unref(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		}
	}

	int LuaPgsqlTask::Await()
	{
		if(this->mRef == 0)
		{
			luaL_error(this->mLua, "not lua coroutine context yield failure");
			return 0;
		}
		return lua_yield(this->mLua, 0);
	}

	inline void lua_push_value(lua_State * L, const char * key, bool value)
	{
		lua_pushstring(L, key);
		lua_pushboolean(L, value);
		lua_rawset(L, -3);
	}

	inline void lua_push_value(lua_State * L, const char * key, unsigned int value)
	{
		lua_pushstring(L, key);
		lua_pushinteger(L, value);
		lua_rawset(L, -3);
	}
	inline void lua_push_value(lua_State * L, const char * key, const std::string & value)
	{
		lua_pushstring(L, key);
		lua_pushlstring(L, value.c_str(), value.size());
		lua_rawset(L, -3);
	}

	void LuaPgsqlTask::OnResponse(std::unique_ptr<pgsql::Response> response) noexcept
	{
		lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		lua_State* coroutine = lua_tothread(this->mLua, -1);

		lua_newtable(this->mLua);

		if (response->mError.empty())
		{
			lua_push_value(this->mLua, "ok", true);
			if (!response->mCmd.empty())
			{
				lua_pushstring(this->mLua, response->mCmd.c_str());
				lua_pushinteger(this->mLua, response->count);
				lua_rawset(this->mLua, -3);
			}
			if (!response->mResults.empty())
			{
				int index = 1;
				lua_pushstring(this->mLua, "result");
				lua_createtable(this->mLua, 0, response->mResults.size());
				for (const std::string& json: response->mResults)
				{
					lua::yyjson::write(this->mLua, json.c_str(), json.size());
					lua_seti(this->mLua, -2, index++);
				}
				lua_rawset(this->mLua, -3);
			}
		}
		else
		{
			lua_push_value(this->mLua, "ok", false);
			lua_push_value(this->mLua, "error", response->mError);
		}
		Lua::Coroutine::Resume(coroutine, 1);
	}
}
