#pragma once;
#include"Lua/Engine/Define.h"
namespace Lua
{
	namespace LuaMongo
	{
		int Make(lua_State* lua);
		int Exec(lua_State* lua);
		int Query(lua_State* lua);
		int QueryOnce(lua_State* lua);
		int Invoke(lua_State* lua, int method);
	}
}