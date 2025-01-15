//
// Created by 64658 on 2025/1/13.
//

#ifndef APP_LIB_H
#define APP_LIB_H

#include "Lua/Engine/LuaInclude.h"
namespace Lua
{
	namespace lfmt
	{
		extern int format(lua_State * L);
		extern int serialize(lua_State * L);
		extern int deserialize(lua_State * L);
	}
}


#endif //APP_LIB_H
