#pragma once

#include"TimerBase.h"
#include<Script/LuaInclude.h>

namespace Sentry
{
    class LuaSleepTimer : public TimerBase
    {
    public:
        LuaSleepTimer(lua_State *lua, int ref, long long ms);

        ~LuaSleepTimer() { luaL_unref(mLuaEnv, LUA_REGISTRYINDEX, this->mRef); }

        static shared_ptr<LuaSleepTimer> Create(lua_State *lua, int index, long long ms);

    public:
        bool Invoke() override; //����֮��ִ�еĲ���
    private:
        int mRef;
        lua_State *mLuaEnv;
    };
}