#pragma once
#include<Script/LuaInclude.h>
namespace SystemExtension
{
	extern int Call(lua_State * luaEnv);

	extern int CallWait(lua_State * luaEnv);

	extern int WaitFor(lua_State * luaEnv);

	extern int WaitNetFrame(lua_State * luaEnv);

	extern int AddTimer(lua_State * lua);

	extern int RemoveTimer(lua_State * lua);

	extern int Start(lua_State * lua);

	extern int Sleep(lua_State * luaEnv);

	extern int GetApp(lua_State * luaEnv);

	extern int GetManager(lua_State * luaEnv);

	extern int SendByAddress(lua_State * luaEnv);

	extern int BindAction(lua_State * luaEnv);
}