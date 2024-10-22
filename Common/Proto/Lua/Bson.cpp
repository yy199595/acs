//
// Created by leyi on 2023/12/14.
//w

#include"Bson.h"
#include"Yyjson/Lua/ljson.h"
#include"Proto/Document/BsonDocument.h"

namespace lua
{
	void encode_once(lua_State* L, int index, bson::Writer::Array& document)
	{
		lua_pushnil(L);
		while (lua_next(L, index) != 0)
		{

		}
	}

	void encode_once(lua_State* L, int index, const char* key, bson::Writer::Document& document)
	{
		switch (lua_type(L, index))
		{
		case LUA_TNUMBER:
		{
			if (lua_isinteger(L, index))
			{
				long long num = lua_tointeger(L, index);
				if (num >= std::numeric_limits<int>::max())
				{
					document.Add(key, num);
					return;
				}
				document.Add(key, (int)num);
				return;
			}
			double num = lua_tonumber(L, index);
			if (num >= std::numeric_limits<float>::max())
			{
				document.Add(key, num);
				return;
			}
			document.Add(key, (float)num);
			break;
		}
		case LUA_TSTRING:
		{
			size_t len = 0;
			const char* str = luaL_tolstring(L, index, &len);
			document.Add(key, str, len);
			break;
		}
		case LUA_TTABLE:
		{
			if (Lua::is_lua_array(L, index))
			{
				bson::Writer::Array doc;
				encode_once(L, index, doc);
				document.Add(key, doc);
				return;
			}
			bson::Writer::Document docObj;

			lua_pushnil(L);
			while (lua_next(L, index) != 0)
			{
				const char* key = lua_tostring(L, -2);
				encode_once(L, -1, key, docObj);
				lua_pop(L, 1);
			}
			break;
		}
		case LUA_TBOOLEAN:
		{
			bool value = lua_toboolean(L, index);
			document.Add(key, value);
			break;
		}
		}
	}

	int lbson::decode(lua_State* L)
	{
		const char * bson = luaL_checkstring(L, 1);
		bson::Reader::Document document(bson);

		return 0;
	}

	int lbson::encode(lua_State* L)
	{
		if (!lua_istable(L, 1))
		{
			luaL_error(L, "must type table");
			return 0;
		}
		bson::Writer::Document document;
		{
			lua_pushnil(L);
			while (lua_next(L, 1) != 0)
			{
				const char* key = lua_tostring(L, -2);
				encode_once(L, -1, key, document);
				lua_pop(L, 1);
			}
			int len = 0;
			const char* b = document.Serialize(len);
			lua_pushlstring(L, b, len);
		}
		return 1;
	}
}