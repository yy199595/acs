#pragma once
#include<Script/LuaInclude.h>

namespace SoEasy
{
	extern int InvokeMysqlCommand(lua_State * lua);
	extern int InvokeRedisCommand(lua_State * lua);
}
