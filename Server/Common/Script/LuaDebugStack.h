#pragma once
#include "LuaInclude.h"
namespace LuaDebugStack
{
	inline void printError(lua_State *L, const char* fmt, ...)
	{
		char text[4096];

		va_list args;
		va_start(args, fmt);
		vsnprintf(text, sizeof(text), fmt, args);
		va_end(args);
		printf("[lua error]  %s\n", text);
		lua_getglobal(L, "_ALERT");
		if (lua_isfunction(L, -1))
		{
			lua_pushstring(L, text);
			lua_call(L, 1, 0);
		}
		else
		{
			lua_pop(L, 1);
		}
	}

	inline int PrintErrorInfo(lua_State * luaEnv)
	{
		lua_getglobal(luaEnv, "debug");  
		lua_getfield(luaEnv, -1, "traceback");  
		lua_pcall(luaEnv, 0, 1, 0);
		const char * str = lua_tostring(luaEnv, -1);
		printf("[Lua Error] : %s\n", str);
		return 0;
	}
	inline void PrintLuaLog(const std::string log)
	{
		printf("[Lua Error] : %s", log.c_str());
	}
	inline void DebugStackData(lua_State * L)
	{
		int top = lua_gettop(L);
		printError(L, "%s", "----------stack----------");
		printError(L, "Type:%s", lua_typename(L, top));
		for (int i = 1; i <= lua_gettop(L); ++i)
		{
			switch (lua_type(L, i))
			{
			case LUA_TNIL:
				printError(L, "\t%s", lua_typename(L, lua_type(L, i)));
				break;
			case LUA_TBOOLEAN:
				printError(L, "\t%s    %s", lua_typename(L, lua_type(L, i)), lua_toboolean(L, i) ? "true" : "false");
				break;
			case LUA_TLIGHTUSERDATA:
				printError(L, "\t%s    0x%08p", lua_typename(L, lua_type(L, i)), lua_topointer(L, i));
				break;
			case LUA_TNUMBER:
				printError(L, "\t%s    %f", lua_typename(L, lua_type(L, i)), lua_tonumber(L, i));
				break;
			case LUA_TSTRING:
				printError(L, "\t%s    %s", lua_typename(L, lua_type(L, i)), lua_tostring(L, i));
				break;
			case LUA_TTABLE:
				printError(L, "\t%s    0x%08p", lua_typename(L, lua_type(L, i)), lua_topointer(L, i));
				break;
			case LUA_TFUNCTION:
				printError(L, "\t%s()  0x%08p", lua_typename(L, lua_type(L, i)), lua_topointer(L, i));
				break;
			case LUA_TUSERDATA:
				printError(L, "\t%s    0x%08p", lua_typename(L, lua_type(L, i)), lua_topointer(L, i));
				break;
			case LUA_TTHREAD:
				printError(L, "\t%s", lua_typename(L, lua_type(L, i)));
				break;
			}
		}
		printError(L, "%s", "-------------------------");
	}

	inline std::string GetCurrentStack(lua_State * luaEnv)
	{
		int level = 1;
		lua_Debug debugInfo;
		std::string retstring;
		while(lua_getstack(luaEnv, level, &debugInfo) == 0)
		{
			lua_getinfo(luaEnv, "", &debugInfo);
			char buffer[1024] = { 0 };
#ifdef _MSC_VER
			size_t size =	sprintf_s(buffer, "%s, %d, %s\n", debugInfo.source, debugInfo.currentline, debugInfo.what);
#else
			size_t size =	sprintf(buffer, "%s, %d, %s\n", debugInfo.source, debugInfo.currentline, debugInfo.what);
#endif // _MSC_VER
			PrintLuaLog(std::string(buffer, size));
			retstring.append(buffer, size);
			level++;
		}
		return retstring;
	}
};