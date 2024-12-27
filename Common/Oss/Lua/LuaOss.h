//
// Created by yy on 2024/10/9.
//

#ifndef APP_LUAOSS_H
#define APP_LUAOSS_H

#include "Lua/Engine/LuaInclude.h"

namespace oss
{
	extern int Sign(lua_State* L);

	extern int Upload(lua_State* L);
}

#endif //APP_LUAOSS_H
