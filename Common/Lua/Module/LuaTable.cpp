//
// Created by yy on 2023/8/19.
//

#include "LuaTable.h"
#include "Proto/Lua/Message.h"
#include "Yyjson/Lua/ljson.h"
namespace lua
{
	LuaTable::LuaTable(lua_State* lua, int count)
		: mLua(lua)
	{
		lua_createtable(lua, 0, count);
	}

	void LuaTable::SetMember(const char* key, const int value)
	{
		lua_pushinteger(this->mLua, value);
		lua_setfield(this->mLua, -2, key);
	}

	void LuaTable::SetMember(const char* key, const long long value)
	{
		lua_pushinteger(this->mLua, value);
		lua_setfield(this->mLua, -2, key);
	}

	void LuaTable::SetMember(const char* key, const std::string& value, bool json)
	{
		do
		{
			if (json)
			{
				lua::yyjson::write(this->mLua, value.c_str(), value.size());
				break;
			}
			lua_pushlstring(this->mLua, value.c_str(), value.size());
		}
		while (false);
		lua_setfield(this->mLua, -2, key);
	}

	void LuaTable::SetMember(const char* key, const pb::Message& value)
	{
		acs::MessageDecoder messageDecoder(this->mLua, nullptr);
		messageDecoder.Decode(value);
	}
}