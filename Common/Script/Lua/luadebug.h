#pragma once

#include"Script/Lua/LuaInclude.h"

namespace Lua
{
	struct LuaDebug
	{
		static void enumStack(lua_State* L);

		static int onError(lua_State* L);

		static void printError(lua_State* L, const char* fmt, ...);

		static void callStack(lua_State* L, int n);
		//static std::map<int, std::string> GetLuaStackData(lua_State * lua);
	};

	inline int LuaDebug::onError(lua_State* L)
	{
		printError(L, "%s", lua_tostring(L, -1));
		lua_getglobal(L, "debug");
		if (lua_istable(L, -1))
		{
			lua_getfield(L, -1, "traceback");
			if (lua_isfunction(L, -1))
			{
				lua_pushstring(L, "Stack trace");
				lua_pcallk(L, 1, 1, 0, 0, 0);
				if (lua_isstring(L, -1))
				{
					std::string error = lua_tostring(L, -1);
					printf("[lua error]  %s\n", error.c_str());
				}
			}
		}
		return 0;
	}

	inline void LuaDebug::printError(lua_State* L, const char* fmt, ...)
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

	inline void LuaDebug::callStack(lua_State* L, int n)
	{
		lua_Debug ar;
		if (lua_getstack(L, n, &ar) == 1)
		{
			lua_getinfo(L, "nSlu", &ar);

			const char* indent;
			if (n == 0)
			{
				indent = "->\t";
				printError(L, "\t<call stack>");
			}
			else
			{
				indent = "\t";
			}

			if (ar.name)
				printError(L, "%s%s() : line %d [%s : line %d]", indent, ar.name, ar.currentline, ar.source,
					ar.linedefined);
			else
				printError(L, "%sunknown : line %d [%s : line %d]", indent, ar.currentline, ar.source, ar.linedefined);

			callStack(L, n + 1);
		}
	}

//inline std::map<int, std::string> LuaDebug::GetLuaStackData(lua_State * lua)
//{
//	std::map<int, std::string> ret;
//
//	int top = lua_gettop(lua);
//	for (int index = 1; index < top + 1; index++)
//	{
//		std::string name = "";
//		switch (lua_type(lua, -index))
//		{
//		case LUA_TNIL:
//			name = "nil";
//			ret.emplace(-index, "nil");
//			break;
//		case LUA_TBOOLEAN:
//			ret.emplace(-index, "nil");
//			break;
//		case LUA_TLIGHTUSERDATA:
//			ret.emplace(-index, "lightuserdata");
//			break;
//		case LUA_TNUMBER:
//			ret.emplace(-index, "number");
//			break;
//		case LUA_TSTRING:
//			ret.emplace(-index, "string");
//			break;
//		case LUA_TTABLE:
//			ret.emplace(-index, "table");
//			break;
//		case LUA_TFUNCTION:
//			ret.emplace(-index, "function");
//			break;
//		case LUA_TUSERDATA:
//			ret.emplace(-index, "userdata");
//			break;
//		case LUA_TTHREAD:
//			ret.emplace(-index, "thread");
//			break;
//		}
//
//		return ret;
//}
//
	inline void LuaDebug::enumStack(lua_State* L)
	{
		//callStack(L, 0);
	}
}
