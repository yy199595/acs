#include "ClassProxyHelper.h"

void ClassProxyHelper::PushGlobalExtensionFunction(lua_State * lua, const char * name, lua_CFunction func)
{
	lua_pushcfunction(lua, func);
	lua_setglobal(lua, name);
}

void ClassProxyHelper::BeginNewTalbe(lua_State * lua, const char * table)
{
	lua_newtable(lua);
	lua_setglobal(lua, table);
}

void ClassProxyHelper::PushStaticExtensionFunction(lua_State * luaEnv, const char * table, const char * name, lua_CFunction func)
{
	lua_getglobal(luaEnv, table);
	if (lua_istable(luaEnv, -1))
	{
		lua_pushstring(luaEnv, name);
		lua_pushcfunction(luaEnv, func);
		lua_rawset(luaEnv, -3);
	}
	lua_pop(luaEnv, 1);
}
