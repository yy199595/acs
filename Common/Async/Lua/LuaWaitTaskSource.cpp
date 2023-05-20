//
// Created by mac on 2022/5/31.
//
#include"LuaWaitTaskSource.h"
#include"XCode/XCode.h"
#include"Entity/Actor/App.h"
#include"Async/Lua/LuaCoroutine.h"
#include"Proto/Component/ProtoComponent.h"
#include<google/protobuf/util/json_util.h>
namespace Tendo
{
	LuaWaitTaskSource::LuaWaitTaskSource(lua_State* lua)
			: mRef(0), mLua(lua)
	{
		if(lua_isthread(this->mLua, -1))
		{
			this->mRef = luaL_ref(lua, LUA_REGISTRYINDEX);
		}
	}
	LuaWaitTaskSource::~LuaWaitTaskSource()
	{
		if(this->mRef != 0)
		{
			luaL_unref(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		}
	}

	int LuaWaitTaskSource::Await()
	{
		if(this->mRef == 0)
		{
			luaL_error(this->mLua, "not lua coroutine context yield failure");
			return 0;
		}
		return lua_yield(this->mLua, 0);
	}

	void LuaWaitTaskSource::SetResult()
	{
		lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
        Lua::Coroutine::Resume(lua_tothread(this->mLua, -1), this->mLua, 0);
	}
}