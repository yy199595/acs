#include "Table.h"
#include <google/protobuf/util/json_util.h>

using namespace google::protobuf::util;

namespace Lua
{
	Table::Table(lua_State* luaEnv, int ref, const std::string& name)
		: ref(ref), mLuaEnv(luaEnv), mTableName(name)
	{

	}

	Table::~Table()
	{
		luaL_unref(mLuaEnv, LUA_REGISTRYINDEX, this->ref);
	}

	std::shared_ptr<Function> Table::GetFunction(const std::string& name)
	{
		lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->ref);
		lua_getfield(this->mLuaEnv, -1, name.c_str());
		if (!lua_isfunction(this->mLuaEnv, -1))
		{
			return nullptr;
		}
		int ref = luaL_ref(this->mLuaEnv, LUA_REGISTRYINDEX);
		return std::make_shared<Function>(this->mLuaEnv, ref);
	}

	std::shared_ptr<Table> Table::GetTable(const std::string& name)
	{
		lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->ref);
		lua_getfield(this->mLuaEnv, -1, name.c_str());
		if (!lua_istable(this->mLuaEnv, -1))
		{
			return nullptr;
		}
		return std::make_shared<Table>(this->mLuaEnv, -1, name);
	}

	std::shared_ptr<Table> Table::Create(lua_State* luaEnv, const std::string& name)
	{
		lua_getglobal(luaEnv, name.c_str());
		if (lua_istable(luaEnv, -1))
		{
			int ref = luaL_ref(luaEnv, LUA_REGISTRYINDEX);
			return std::make_shared<Table>(luaEnv, ref, name);
		}
		return nullptr;
	}

	bool Table::Serialization(std::string& outString)
	{
		return true;
	}

	std::string Table::ToJson()
	{
		if(Lua::Function::Get(this->mLuaEnv, "Json", "ToString"))
		{
			lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->ref);
			if (!lua_istable(this->mLuaEnv, -1))
			{
				return std::string();
			}
			if (lua_pcall(this->mLuaEnv, 1, 1, 0) != 0)
			{
				printf("%s", lua_tostring(this->mLuaEnv, -1));
				return std::string();
			}
			size_t size = 0;
			return std::string(lua_tolstring(this->mLuaEnv, -1, &size), size);
		}
		return std::string();
	}
	bool Table::Get(lua_State * lua, const std::string & name)
	{
		lua_getglobal(lua, name.c_str());
		return lua_istable(lua, -1);
	}
}
