#include"DelayTimer.h"

namespace acs
{
	void LuaTimer::Invoke()
	{
		lua_settop(this->mLua, 0);
		lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->ref);
		switch (lua_type(this->mLua, -1))
		{
			case LUA_TFUNCTION:
			{
				if (lua_pcall(this->mLua, 0, 0, 0) != LUA_OK)
				{
					luaL_error(this->mLua, "[call function ] = %s", lua_tostring(this->mLua, -1));
				}
				break;
			}
			case LUA_TTABLE:
			{
				lua_getfield(this->mLua, -1, this->mMethod.c_str());
				if (lua_isfunction(this->mLua, -1))
				{
					lua_pushvalue(this->mLua, -2);
					if (lua_pcall(this->mLua, 1, 0, 0) != LUA_OK)
					{
						luaL_error(this->mLua, "[call function ] = %s", lua_tostring(this->mLua, -1));
					}
				}
				break;
			}
			default:
				luaL_error(this->mLua, "unknown lua type");
				break;
		}
	}
}