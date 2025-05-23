#include"DelayTimer.h"
#include"Log/Common/CommonLogDef.h"
namespace acs
{
	LuaTimer::~LuaTimer() noexcept
	{
		luaL_unref(this->mLua, LUA_REGISTRYINDEX, ref);
		if(this->cor > 0)
		{
			luaL_unref(this->mLua, LUA_REGISTRYINDEX, cor);
		}
	}

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
					LOG_ERROR("{}", lua_tostring(this->mLua, -1));
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
						LOG_ERROR("{}", lua_tostring(this->mLua, -1));
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