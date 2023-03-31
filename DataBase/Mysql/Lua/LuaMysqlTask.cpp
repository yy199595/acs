//
// Created by yjz on 2023/3/24.
//

#ifdef __ENABLE_MYSQL__

#include"LuaMysqlTask.h"
#include"Script/Lua/LuaInclude.h"
#include"Util/Json/Lua/Json.h"

namespace Sentry
{
	LuaMysqlTask::LuaMysqlTask(lua_State *lua, int id, int method)
		: IRpcTask<Mysql::Response>(id), mLua(lua)
	{
		this->mRef = 0;
		this->mMethod = method;
		if(lua_isthread(this->mLua, -1))
		{
			this->mRef = luaL_ref(lua, LUA_REGISTRYINDEX);
		}
	}
	LuaMysqlTask::~LuaMysqlTask()
	{
		if(this->mRef != 0)
		{
			luaL_unref(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		}
	}

	int LuaMysqlTask::Await()
	{
		if(this->mRef == 0)
		{
			luaL_error(this->mLua, "not lua coroutine context yield failure");
			return 0;
		}
		return lua_yield(this->mLua, 0);
	}

	void LuaMysqlTask::OnResponse(std::shared_ptr<Mysql::Response> response)
	{
		lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		lua_State* coroutine = lua_tothread(this->mLua, -1);
		switch (this->mMethod)
		{
			case LUA_MYSQL_EXEC:
			{
				lua_pushboolean(this->mLua, response->IsOk());
			}
				break;
			case LUA_MYSQL_QUERY:
			{
				if (response->ArraySize() > 0)
				{
					int count = (int)response->ArraySize();
					lua_createtable(this->mLua, 0, count);
					for (int index = 0; index < response->ArraySize(); index++)
					{
						lua_pushinteger(this->mLua, index + 1);
						Lua::RapidJson::Write(this->mLua, response->Get(index));
						lua_settable(this->mLua, -3);
					}
				}
				else
				{
					lua_pushnil(this->mLua);
				}
			}
				break;
			case LUA_MYSQL_QUERY_ONE:
			{
				if (response->ArraySize() > 0)
				{
					Lua::RapidJson::Write(this->mLua, response->Get(0));
				}
				else
				{
					lua_pushnil(this->mLua);
				}
			}
				break;
		}
		Lua::Coroutine::Resume(coroutine, this->mLua, 1);
	}
}

#endif