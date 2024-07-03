//
// Created by yy on 2024/1/1.
//

#include"Auth.h"
#include"Auth/Jwt/Jwt.h"
#include"Yyjson/Lua/ljson.h"
namespace lua
{
	int ljwt::Create(lua_State* L)
	{
		std::string data;
		std::string key = luaL_checkstring(L, 1);
		if (lua_isstring(L, 2))
		{
			data = lua_tostring(L, 2);
		}
		else if (lua_istable(L, 2))
		{
			if(!yyjson::read(L, 2, data))
			{
				return 0;
			}
		}
#ifdef __ENABLE_OPEN_SSL__
		std::string token = jwt::Create(data, key);
		lua_pushlstring(L, token.c_str(), token.size());
#else
		lua_pushnil(L);
#endif
		return 1;
	}

	int ljwt::Verify(lua_State* L)
	{
#ifdef __ENABLE_OPEN_SSL__
		size_t len = 0;
		std::string data;
		const char * token = luaL_checklstring(L, 1, &len);
		const std::string key = luaL_checkstring(L, 2);
		if(!jwt::Verify(token, key, data))
		{
			return 0;
		}
		return yyjson::write(L, data.c_str(), data.size()) ? 1: 0;
#endif
		lua_pushnil(L);
		return 1;
	}
}