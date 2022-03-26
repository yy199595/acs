#include "LuaSleepTimer.h"
#include"App/App.h"
#include <Define/CommonLogDef.h>
namespace Sentry
{
    LuaSleepTimer::LuaSleepTimer(lua_State *lua, int ref, long long ms)
        : TimerBase(ms)
    {
        this->mRef = ref;
        this->mLuaEnv = lua;
    }

    std::shared_ptr<LuaSleepTimer> LuaSleepTimer::Create(lua_State *lua, int index, long long ms)
    {
        if (!lua_isthread(lua, index))
        {
            return nullptr;
        }
        int ref = luaL_ref(lua, LUA_REGISTRYINDEX);
        return std::make_shared<LuaSleepTimer>(lua, ref, ms);
    }

    void LuaSleepTimer::Invoke(TimerState state)
    {
        lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->mRef);
        if (!lua_isthread(this->mLuaEnv, -1))
        {
            LOG_ERROR("invoke lua sleep timer error");
            return;
        }
        lua_State *co = lua_tothread(this->mLuaEnv, -1);
		lua_presume(co, this->mLuaEnv, 0);
    }
}// namespace Sentry
