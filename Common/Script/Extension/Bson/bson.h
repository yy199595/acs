//
// Created by yjz on 2022/5/20.
//

#ifndef _BSON_H_
#define _BSON_H_
#include"Script/LuaInclude.h"

namespace luabson
{
	extern int lencode(lua_State* luaState);
	extern int lencode_order(lua_State* luaState);
	extern int ldate(lua_State* luaState);
	extern int ltimestamp(lua_State* luaState);
	extern int lregex(lua_State* luaState);
	extern int lbinary(lua_State* luaState);
	extern int lobjectid(lua_State* luaState);
	extern int ldecode(lua_State* luaState);
}
#endif //_BSON_H_
