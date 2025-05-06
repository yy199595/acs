//
// Created by 64658 on 2025/4/9.
//

#pragma once
#include "Lua/Engine/Define.h"

namespace lua
{
	namespace node
	{
		extern int Next(lua_State * L);
		extern int Rand(lua_State * L);
		extern int Hash(lua_State * L);

		extern int Query(lua_State * L);
		extern int Create(lua_State * l);
		extern int Remove(lua_State * L);
		extern int AllInfo(lua_State * l);
		extern int GetListen(lua_State * l);
		extern int AddListen(lua_State * l);
	}
}

