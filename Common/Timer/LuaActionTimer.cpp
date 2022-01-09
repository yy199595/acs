#include "LuaActionTimer.h"
#include <Define/CommonLogDef.h>
#include <Util/MathHelper.h>
#include"Core/App.h"
namespace GameKeeper
{
    LuaActionTimer::LuaActionTimer(lua_State *luaEnv, int ref, int interval, int count)
        : TimerBase(interval)
    {
        this->mRef = ref;
        this->mLuaEnv = luaEnv;
        this->mInterval = interval;
        this->mInvokeCount = 0;
        this->mMaxInvokeCount = count;
    }

    LuaActionTimer::~LuaActionTimer()
    {
        luaL_unref(mLuaEnv, LUA_REGISTRYINDEX, this->mRef);
    }

    bool LuaActionTimer::Invoke()
    {
        lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->mRef);
        if (!lua_isfunction(this->mLuaEnv, -1))
        {
            return true;
        }
        if (lua_pcall(this->mLuaEnv, 0, 0, 0) != 0)
        {
			std::cerr << lua_tostring(this->mLuaEnv, -1) << std::endl;
            return true;
        }
        this->mInvokeCount++;
        if (this->mMaxInvokeCount == -1)
        {
            return false;
        }
        return this->mInvokeCount >= this->mMaxInvokeCount;
    }
}// namespace GameKeeper
