#pragma once
#include<Script/LuaInclude.h>
namespace SoEasy
{
	class SystemExtension
	{
	public:
		static int Call(lua_State * luaEnv);

		static int CallWait(lua_State * luaEnv);

		static int CallAction(lua_State * luaEnv);

		static int AddTimer(lua_State * lua);

		static int RemoveTimer(lua_State * lua);

		static int Start(lua_State * lua);

		static int Sleep(lua_State * luaEnv);

		static int GetApp(lua_State * luaEnv);

		static int GetManager(lua_State * luaEnv);

		static int SendByAddress(lua_State * luaEnv);

		static int BindAction(lua_State * luaEnv);
	};
}