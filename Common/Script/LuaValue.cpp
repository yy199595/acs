//
// Created by mac on 2022/3/31.
//

#include "LuaValue.h"

LuaValue::LuaValue(lua_State* lua, int index)
{
	lua_pushvalue(lua, index);
	this->mRef = luaL_ref(lua, LUA_REGISTRYINDEX);
}

LuaValue::~LuaValue()
{
	luaL_unref(this->mLua, LUA_REGISTRYINDEX, this->mRef);
}

