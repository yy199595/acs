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

	void ModuleClass::Register(const luaL_Reg& luaLib)
	{
		luaL_requiref(this->mLua, luaLib.name, luaLib.func, 1);
		lua_pop(this->mLua, 1);
	}

	void ModuleClass::Register(const char* module, lua_CFunction func)
	{
		luaL_requiref(this->mLua, module, func, 1);
		lua_pop(this->mLua, 1);
	}
}