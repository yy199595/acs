//
// Created by yjz on 2023/3/24.
//

#ifdef __ENABLE_MYSQL__

#include"LuaMysqlTask.h"
#include"Script/Lua/LuaInclude.h"
#include"Util/Json/Lua/Json.h"

namespace Tendo
{
	LuaMysqlTask::LuaMysqlTask(lua_State *lua, int id)
		: IRpcTask<Mysql::Response>(id), mLua(lua)
	{
		this->mRef = 0;
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
		
		int count = response->WriteToLua(this->mLua);
		Lua::Coroutine::Resume(coroutine, this->mLua, count);
	}
}

#endif