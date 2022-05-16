//
// Created by mac on 2022/5/16.
//

#include"Json.h"
#include"rapidjson/rapidjson.h"
#include"Encoder.h"
namespace Lua
{
	int Json::Encode(lua_State* lua)
	{
		try{
			Encoder encode(lua, 2);
			StringBuffer s;
			encode.encode(lua, &s, 1);
			lua_pushlstring(lua, s.GetString(), s.GetSize());
			return 1;
		}
		catch (...) {
			luaL_error(lua, "error while encoding");
		}
		return 0;
	}

	int Json::Decode(lua_State* lua)
	{
		size_t len = 0;
		const char*  contents = nullptr;
		switch(lua_type(lua, 1)) {
		case LUA_TSTRING:
			contents = luaL_checklstring(lua, 1, &len);
			break;
		case LUA_TLIGHTUSERDATA:
			contents = reinterpret_cast<const char*>(lua_touserdata(lua, 1));
			len = luaL_checkinteger(lua, 2);
			break;
		default:
			return luaL_argerror(lua, 1, "required string or lightuserdata (points to a memory of a string)");
		}

		rapidjson::extend::StringStream s(contents, len);
		return values::pushDecoded(lua, s);
	}
}