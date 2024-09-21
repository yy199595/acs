#pragma once

#include"Lua/Engine/Function.h"
namespace Lua
{
	class Table
	{
	 public:
		Table(lua_State* luaEnv, int ref);

		~Table();

	 public:
		static bool Get(lua_State * lua, const std::string & name);
		static std::unique_ptr<Table> Create(lua_State* luaEnv, const std::string& name);

	 public:
		inline int Length();
		template<typename T>
		inline T GetMember(const char* name);


	 public:
		std::unique_ptr<Table> GetTable(int index);
		std::unique_ptr<Table> GetTable(const std::string& name);
		std::unique_ptr<Function> GetFunction(const std::string& name);

	 private:
		int ref;
		lua_State* mLuaEnv;
	};

	int Table::Length()
	{
		lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->ref);
		if (!lua_istable(this->mLuaEnv, -1))
		{
			return 0;
		}
		lua_len(this->mLuaEnv, -1);
		return lua_tointeger(this->mLuaEnv, -1);
	}

	template<typename T>
	T Table::GetMember(const char* name)
	{
		lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->ref);
		if (lua_istable(this->mLuaEnv, -1))
		{
			lua_getfield(this->mLuaEnv, -1, name);
			return Parameter::Read<T>(this->mLuaEnv, -1);
		}
		return T();
	}
}
