//
// Created by mac on 2022/3/31.
//

#ifndef SERVER_LUAVALUE_H
#define SERVER_LUAVALUE_H
#include"LuaParameter.h"

namespace Lua
{
	class LuaValue
	{
	 public:
		LuaValue(lua_State* lua, int index);
		~LuaValue();
	 private:
		template<typename T>
		T GetValue();
	 private:
		int mRef;
		lua_State* mLua;
	};

	template<typename T>
	T LuaValue::GetValue()
	{
		lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		return Parameter::Read<T>(this->mLua, -1);
	}
#endif //SERVER_LUAVALUE_H
}
