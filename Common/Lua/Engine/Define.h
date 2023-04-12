//
// Created by MyPC on 2023/4/12.
//

#ifndef APP_DEFINE_H
#define APP_DEFINE_H

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
};


class ILuaWrite
{
public:
	virtual int WriteToLua(lua_State* lua) = 0;
};

#endif //APP_DEFINE_H
