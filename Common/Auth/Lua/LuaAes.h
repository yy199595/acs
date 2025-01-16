//
// Created by 64658 on 2025/1/16.
//

#ifndef APP_LUAAES_H
#define APP_LUAAES_H
#include "Lua/Engine/LuaInclude.h"
namespace lua
{
	namespace laes
	{
		extern int Encode(lua_State * L);
		extern int Decode(lua_State * L);
	}
}



#endif //APP_LUAAES_H
