//
// Created by yjz on 2022/5/23.
//

#include"LuaService.h"
using namespace Sentry;
namespace Lua
{
	int Service::Call(lua_State* lua)
	{
		if(lua_isuserdata(lua, 1))
		{
			ServiceCallComponent * callComponent = (ServiceCallComponent*)lua_touserdata(lua, 1);
			if (lua_isinteger(lua, 2)) //userId
			{
				long long userId = lua_tointeger(lua, 2);
				std::string func = lua_tostring(lua, 3);
				if(lua_isstring(lua, 4) && lua_isstring(lua, 5))
				{
					size_t size = 0;
					std::string proto = lua_tostring(lua, 4);
					const char * json = lua_tolstring(lua, 5, & size);
				}
			}
			else if (lua_isstring(lua, 2)) //address
			{
				std::string address(lua_tostring(lua, 1));
			}
			luaL_error(lua, "call paremeter two is userId or address");
			return 0;
		}
		luaL_error(lua, "call paremeter first is service");
		return 0;
	}
}