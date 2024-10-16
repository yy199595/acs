//
// Created by 64658 on 2024/10/16.
//

#include <vector>
#include "LuaString.h"
#include "fmt.h"
#include "bundled/core.h"
#include "bundled/args.h"

namespace Lua
{
	int Str::Format(lua_State* L)
	{
		if (!lua_isstring(L, 1)) {
			lua_pushstring(L, "First argument must be a string");
			lua_error(L);
		}
		const char* fmt_str = lua_tostring(L, 1);

		return 1;
	}
}