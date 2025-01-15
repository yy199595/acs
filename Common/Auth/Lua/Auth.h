//
// Created by yy on 2024/1/1.
//

#ifndef APP_AUTH_H
#define APP_AUTH_H
#include"Lua/Lib/Lib.h"
namespace lua
{
	namespace ljwt
	{
		extern int Create(lua_State *);
		extern int Verify(lua_State *);
	}
}
#endif //APP_AUTH_H
