//
// Created by mac on 2022/3/31.
//

#include"WaitLuaTaskSource.h"
namespace acs
{
	WaitLuaTaskSource::WaitLuaTaskSource()
	{
		this->ref = 0;
		this->mLua = nullptr;
	}

	WaitLuaTaskSource::~WaitLuaTaskSource()
	{
		if(this->ref > 0)
		{
			lua_unref(this->mLua, this->ref);
		}
	}

	int WaitLuaTaskSource::SetResult(lua_State* lua)
	{
		WaitLuaTaskSource* luaTaskSource = Lua::PtrProxy<WaitLuaTaskSource>::Read(lua, 1);

		if(luaTaskSource != nullptr && luaTaskSource->ResumeTask())
		{
			luaTaskSource->mLua = lua;
			luaTaskSource->ref = luaL_ref(lua, LUA_REGISTRYINDEX);
		}
		return 0;
	}
}

