//
// Created by 64658 on 2025/6/16.
//

#ifndef APP_LUACENSOR_H
#define APP_LUACENSOR_H
#include"Lua/Lib/Lib.h"
namespace censor
{
	extern int New(lua_State * L);
	extern int Load(lua_State * L);
	extern int Mask(lua_State * L);
	extern int Check(lua_State * L);
	extern int Build(lua_State * L);
	extern int Insert(lua_State * L);
}


#endif //APP_LUACENSOR_H
