//
// Created by mac on 2022/5/16.
//

#include"Json.h"
#include"rapidjson/rapidjson.h"
#include"Encoder.h"
namespace Lua
{
    void RapidJson::Read(lua_State *lua, int index, std::string *json)
    {
        StringBuffer s;
        Encoder encode(lua, index);
        encode.encode(lua, &s, index);
        json->append(s.GetString(), s.GetLength());
    }

    void RapidJson::Write(lua_State *lua, const std::string &json)
    {
        values::pushDecoded(lua, json.c_str(), json.size());
    }

	int RapidJson::Encode(lua_State* lua)
	{
		try
        {
            StringBuffer s;
            Encoder encode(lua, 2);
			encode.encode(lua, &s, 1);
			lua_pushlstring(lua, s.GetString(), s.GetSize());
			return 1;
		}
		catch (...) {
			luaL_error(lua, "error while encoding");
		}
		return 0;
	}

	int RapidJson::Decode(lua_State* lua)
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
		return values::pushDecoded(lua, contents, len);
	}
}