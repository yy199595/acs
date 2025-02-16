//
// Created by yy on 2024/1/1.
//

#include"Auth.h"
#ifdef __ENABLE_OPEN_SSL__
#include"Auth/Jwt/Jwt.h"
#else
#include "Util/Crypt/Mask.h"
#include "Proto/Bson/base64.h"
#endif
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
		help::Mask::Encode(data, key);
		std::string token = _bson::base64::encode(data);
		lua_pushlstring(L, token.c_str(), token.size());
#endif
		return 1;
	}

	int ljwt::Verify(lua_State* L)
	{
		size_t len = 0;
		std::string data;
		const char * token = luaL_checklstring(L, 1, &len);
		const std::string key = luaL_checkstring(L, 2);
#ifdef __ENABLE_OPEN_SSL__
		if(!jwt::Verify(token, key, data))
		{
			return 0;
		}
#else
		data = _bson::base64::decode(token);
		help::Mask::Decode(data, key);
#endif
		return yyjson::write(L, data.c_str(), data.size()) ? 1: 0;
	}
}
