//
// Created by leyi on 2023/5/8.
//

#include "LocalClassProxy.h"

namespace Lua
{
	void LocalClassProxy::NewTable(const std::string& name, int count)
	{
		lua_createtable(this->mLua, 0, count);
	}

	void LocalClassProxy::PushMemberFunction(const std::string& name, lua_CFunction func)
	{
		lua_pushcfunction(this->mLua, func);
		lua_setfield(this->mLua, -2, name.c_str());
	}
}