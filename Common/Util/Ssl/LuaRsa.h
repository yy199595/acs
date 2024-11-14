//
// Created by 64658 on 2024/11/7.
//

#ifndef APP_LUARSA_H
#define APP_LUARSA_H
#ifdef __ENABLE_OPEN_SSL__
#include "Lua/Engine/LuaInclude.h"
namespace Lua
{
	namespace rsa
	{
		extern int Init(lua_State *);
		extern int Encode(lua_State *);
		extern int Decode(lua_State *);
	}
}
#endif


#endif //APP_LUARSA_H
