//
// Created by 64658 on 2025/1/13.
//

#ifndef APP_LIB_H
#define APP_LIB_H

#include "Lua/Engine/LuaInclude.h"
namespace lua
{
	namespace lfmt
	{
		extern int format(lua_State * L);
		extern int serialize(lua_State * L);
		extern int deserialize(lua_State * L);
	}

	namespace lib
	{
		extern int luaopen_lfs(lua_State* L);
		extern int luaopen_ljwt(lua_State* L);
		extern int luaopen_lfmt(lua_State* L);
		extern int luaopen_lmd5(lua_State* L);
		extern int luaopen_lapp(lua_State* L);
		extern int luaopen_ljson(lua_State* L);
		extern int luaopen_lbson(lua_State * L);
		extern int luaopen_lbase64(lua_State* L);
		extern int luaopen_lmonogodb(lua_State* L);

	}
}


#endif //APP_LIB_H
