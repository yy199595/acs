//
// Created by leyi on 2023/6/26.
//

#include "ModuleClass.h"

namespace Lua
{
	ModuleClass::ModuleClass(lua_State* lua)
		: mLua(lua)
	{

	}

	ModuleClass & ModuleClass::Start()
	{
		lua_settop(this->mLua, 1);
		lua_getfield(this->mLua, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);

		lua_newtable(this->mLua);
		return *this;
	}

	ModuleClass & ModuleClass::AddFunction(const char* name, lua_CFunction func)
	{
		lua_pushstring(this->mLua, name);
		lua_pushcfunction(this->mLua, func);
		lua_rawset(this->mLua, -3);
		return  *this;
	}
}