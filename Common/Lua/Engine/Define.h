//
// Created by MyPC on 2023/4/12.
//

#pragma once;

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
};


class ILuaWrite
{
public:
	virtual int WriteToLua(lua_State* lua) const = 0;
};
