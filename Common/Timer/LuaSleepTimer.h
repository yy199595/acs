#pragma once

#include "TimerBase.h"
#include <Script/LuaInclude.h>

namespace Sentry
{
    class LuaSleepTimer : public TimerBase
    {
    public:
        LuaSleepTimer(lua_State *lua, int ref, long long ms);

        ~LuaSleepTimer() final { luaL_unref(mLuaEnv, LUA_REGISTRYINDEX, this->mRef); }

        static LuaSleepTimer * Create(lua_State *lua, int index, long long ms);

    public:
        void Invoke() override;
    private:
        int mRef;
        lua_State *mLuaEnv;
    };
}// namespace Sentry