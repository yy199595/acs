#pragma once

#include <Script/LuaInclude.h>

namespace Lua
{
	namespace CoroutineExtension
	{
		extern int Sleep(lua_State* lua);

		extern int Start(lua_State* lua);
	}// namespace CoroutineExtension
}