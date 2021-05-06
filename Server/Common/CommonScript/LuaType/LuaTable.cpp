#include "LuaTable.h"
#include<google/protobuf/util/json_util.h>
using namespace google::protobuf::util;
LuaTable::LuaTable(lua_State * luaEnv, int ref)
	: ref(ref), luaEnv(luaEnv) { }

LuaTable::LuaTable(lua_State * luaEnv, int index, bool isPos)
{
	if (lua_istable(luaEnv, index))
	{
		lua_pushvalue(luaEnv, index);
		this->luaEnv = luaEnv;
		this->ref = luaL_ref(luaEnv, LUA_REGISTRYINDEX);
	}
}


LuaTable::~LuaTable()
{
	luaL_unref(luaEnv, LUA_REGISTRYINDEX, this->ref);
}

LuaTable * LuaTable::Create(lua_State * luaEnv, const std::string name)
{
	lua_getglobal(luaEnv, name.c_str());
	if (lua_istable(luaEnv, -1))
	{
		int ref = luaL_ref(luaEnv, LUA_REGISTRYINDEX);
		LuaTable * pLuaTable = new LuaTable(luaEnv, ref);
		pLuaTable->tableName = name;
		return pLuaTable;
	}
	return nullptr;
}

bool LuaTable::Serialization(std::string & outString)
{
	outString.clear();
	lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->ref);
	if (!lua_istable(this->luaEnv, -1))
	{
		return false;
	}
	if (lua_getfunction(this->luaEnv, "JsonUtil", "ToString"))
	{
		const char * t1 = lua_typename(this->luaEnv, -1);
		const char * t2 = lua_typename(this->luaEnv, -2);

		lua_pushvalue(this->luaEnv, -2);
		const char * t3 = lua_typename(this->luaEnv, -1);
		const char * t4 = lua_typename(this->luaEnv, -2);

		if (lua_pcall(this->luaEnv, 1, 1, 0) != 0)
		{
			printf("%s", lua_tostring(this->luaEnv, -1));
			return false;
		}
		size_t size = 0;
		outString.append(lua_tolstring(this->luaEnv, -1, &size), size);
		return true;
	}
	return false;
}

bool LuaTable::Serialization(Message & message)
{
	message.Clear();
	std::string mMessageBuffer;
	if (!this->Serialization(mMessageBuffer))
	{
		return false;
	}
	return JsonStringToMessage(mMessageBuffer, &message).ok();
}

