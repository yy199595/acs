#pragma once
#include"Script/LuaInclude.h"

namespace Lua
{
	namespace Md5
	{
		int ToString(lua_State* lua);
	}

	namespace Sha1
	{
		int GetHash(lua_State* lua);
		int GetMacHash(lua_State* lua);
	}
}