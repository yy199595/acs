#include "LuaTable.h"
#include <google/protobuf/util/json_util.h>

using namespace google::protobuf::util;

namespace Lua
{
	LuaTable::LuaTable(lua_State* luaEnv, int ref, const std::string& name)
		: ref(ref), mLuaEnv(luaEnv), mTableName(name)
	{

	}

	LuaTable::~LuaTable()
	{
		luaL_unref(mLuaEnv, LUA_REGISTRYINDEX, this->ref);
	}

	std::shared_ptr<LuaFunction> LuaTable::GetFunction(const std::string& name)
	{
		lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->ref);
		lua_getfield(this->mLuaEnv, -1, name.c_str());
		if (!lua_isfunction(this->mLuaEnv, -1))
		{
			return nullptr;
		}
		int ref = luaL_ref(this->mLuaEnv, LUA_REGISTRYINDEX);
		return std::make_shared<LuaFunction>(this->mLuaEnv, ref);
	}

	std::shared_ptr<LuaTable> LuaTable::GetTable(const std::string& name)
	{
		lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->ref);
		lua_getfield(this->mLuaEnv, -1, name.c_str());
		if (!lua_istable(this->mLuaEnv, -1))
		{
			return nullptr;
		}
		return std::make_shared<LuaTable>(this->mLuaEnv, -1, name);
	}

	std::shared_ptr<LuaTable> LuaTable::Create(lua_State* luaEnv, const std::string& name)
	{
		lua_getglobal(luaEnv, name.c_str());
		if (lua_istable(luaEnv, -1))
		{
			int ref = luaL_ref(luaEnv, LUA_REGISTRYINDEX);
			return std::make_shared<LuaTable>(luaEnv, ref, name);
		}
		return nullptr;
	}

	bool LuaTable::Serialization(std::string& outString)
	{
		outString.clear();
		lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->ref);
		if (!lua_istable(this->mLuaEnv, -1))
		{
			return false;
		}
		if (lua_getfunction(this->mLuaEnv, "Json", "ToString"))
		{
			lua_pushvalue(this->mLuaEnv, -2);
			if (lua_pcall(this->mLuaEnv, 1, 1, 0) != 0)
			{
				printf("%s", lua_tostring(this->mLuaEnv, -1));
				return false;
			}
			size_t size = 0;
			const char* json = lua_tolstring(this->mLuaEnv, 01, &size);
			outString.append(json, size);
			return true;
		}
		return false;
	}

	bool LuaTable::Serialization(Message& message)
	{
		message.Clear();
		std::string mMessageBuffer;
		if (!this->Serialization(mMessageBuffer))
		{
			return false;
		}
		return JsonStringToMessage(mMessageBuffer, &message).ok();
	}
}
