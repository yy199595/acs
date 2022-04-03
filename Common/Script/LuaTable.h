#pragma once

#include<Define/CommonLogDef.h>
#include<Script/LuaFunction.h>
#include<google/protobuf/message.h>
using namespace google::protobuf;

namespace Lua
{
	class LuaTable
	{
	 public:
		LuaTable(lua_State* luaEnv, int ref, const std::string& name);

		~LuaTable();

	 public:
		static std::shared_ptr<LuaTable> Create(lua_State* luaEnv, const std::string& name);

	 public:
		template<typename T>
		T GetMemberVariable(const char* name);

	 public:
		int GetRef()
		{
			return this->ref;
		}
		std::shared_ptr<LuaTable> GetTable(const std::string& name);
		std::shared_ptr<LuaFunction> GetFunction(const std::string& name);

		bool Serialization(Message& message);

		bool Serialization(std::string& outString);

	 private:
		int ref;
		lua_State* mLuaEnv;
		std::string mTableName;
	};

	template<typename T>
	T LuaTable::GetMemberVariable(const char* name)
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
