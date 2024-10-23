//
// Created by 64658 on 2024/10/16.
//

#include <vector>
#include "LuaString.h"
#include "bundled/core.h"
#include "bundled/args.h"
#include "Yyjson/Lua/ljson.h"

namespace Lua
{
	int Str::Format(lua_State* L)
	{
		const char* formatStr = luaL_checkstring(L, 1);

		int numArgs = lua_gettop(L) - 1;
		std::vector<fmt::basic_format_arg<fmt::format_context>> args;
		args.reserve(numArgs);
		std::vector<std::string> tempArray;
		for (int i = 2; i <= numArgs + 1; ++i)
		{
			switch(lua_type(L, i))
			{
				case LUA_TNIL:
					args.emplace_back(fmt::detail::make_arg<fmt::format_context>("nil"));
					break;
				case LUA_TNUMBER:
					if(lua_isinteger(L, i)) {
						args.emplace_back(fmt::detail::make_arg<fmt::format_context>((long long)lua_tonumber(L, i)));
					} else {
						args.emplace_back(fmt::detail::make_arg<fmt::format_context>(lua_tonumber(L, i)));
					}
					break;
				case LUA_TSTRING:
					args.emplace_back(fmt::detail::make_arg<fmt::format_context>(lua_tostring(L, i)));
					break;
				case LUA_TBOOLEAN:
					args.emplace_back(fmt::detail::make_arg<fmt::format_context>((bool)lua_toboolean(L, i)));
					break;
				case LUA_TTABLE:
				{
					std::string json;
					lua::yyjson::read(L, i, json);
					tempArray.emplace_back(json);
					const std::string & tmp = tempArray.back();
					args.emplace_back(fmt::detail::make_arg<fmt::format_context>(tmp));
					break;
				}
				default:
				{
					luaL_error(L, "Unsupported argument type:%s", lua_typename(L, i));
					return 0;
				}
			}
		}
		try
		{
			std::string result = fmt::vformat(formatStr,
					fmt::basic_format_args<fmt::format_context>(args.data(), (int)args.size()));
			lua_pushlstring(L, result.c_str(), result.size());
			return 1;
		}
		catch (std::exception & e)
		{
			luaL_error(L, e.what());
			return 0;
		}
	}
}