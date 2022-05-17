//
// Created by mac on 2022/3/31.
//

#include "LuaTaskSource.h"
#include"Script/UserDataParameter.h"
namespace Sentry
{
	LuaTaskSource::LuaTaskSource()
	{
		this->ref = 0;
		this->mLua = nullptr;
	}

	LuaTaskSource::~LuaTaskSource()
	{
		lua_unref(this->mLua, this->ref);
	}

	int LuaTaskSource::SetResult(lua_State* lua)
	{
		LuaTaskSource* luaTaskSource = Lua::PtrProxy<LuaTaskSource>::Read(lua, 1);

		if(luaTaskSource != nullptr && luaTaskSource->ResumeTask())
		{
			luaTaskSource->mLua = lua;
			luaTaskSource->ref = luaL_ref(lua, LUA_REGISTRYINDEX);
		}
		return 0;
	}
}

