//
// Created by leyi on 2023/6/26.
//

#include "ModuleClass.h"

namespace Lua
{
	CCModule::CCModule(lua_State* lua)
		: mLua(lua)
	{

	}

	void CCModule::Open(const luaL_Reg& luaLib)
	{
		luaL_requiref(this->mLua, luaLib.name, luaLib.func, 1);
		lua_pop(this->mLua, 1);
	}

	void CCModule::Open(const char* module, lua_CFunction func)
	{
		luaL_requiref(this->mLua, module, func, 1);
		lua_pop(this->mLua, 1);
	}
}