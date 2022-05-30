#include "ClassProxyHelper.h"

namespace Lua
{
	ClassProxyHelper::ClassProxyHelper(lua_State* lua, const std::string& name)
		: mLua(lua), mName(name)
	{

	}

	void ClassProxyHelper::PushGlobalExtensionFunction(const char* name, lua_CFunction func)
	{
		lua_pushcfunction(this->mLua, func);
		lua_setglobal(this->mLua, name);
	}

	void ClassProxyHelper::BeginNewTable()
	{
		lua_getglobal(this->mLua, this->mName.c_str());
		if (lua_isnil(this->mLua, -1))
		{
			lua_newtable(this->mLua);
			{
				lua_pushstring(this->mLua, "__index");
				lua_pushcclosure(this->mLua, ClassMateProxy::OnMetaTable, 0);
				lua_rawset(this->mLua, -3);
			}
			lua_setglobal(this->mLua, this->mName.c_str());
		}
	}

	void ClassProxyHelper::PushStaticExtensionFunction(const char* name, lua_CFunction func)
	{
		lua_getglobal(this->mLua, this->mName.c_str());
		if (lua_isnil(this->mLua, -1))
		{
			lua_newtable(this->mLua);
			lua_setglobal(this->mLua, this->mName.c_str());
		}
		lua_getglobal(this->mLua, this->mName.c_str());
		if (lua_istable(this->mLua, -1))
		{
			lua_pushstring(this->mLua, name);
			lua_pushcfunction(this->mLua, func);
			lua_rawset(this->mLua, -3);
		}
		lua_pop(this->mLua, 1);
	}

	ClassProxyHelper ClassProxyHelper::Clone(const string& name)
	{
		return ClassProxyHelper(this->mLua, name);
	}
}
