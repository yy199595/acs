#pragma once

#include "TimerBase.h"
#include <Script/LuaInclude.h>

namespace Sentry
{
    class LuaActionTimer : public TimerBase
    {
    public:
        LuaActionTimer(lua_State *luaEnv, int ref, int interval, int count = 1);

        ~LuaActionTimer();

    public:
        bool Invoke() override;//����֮��ִ�еĲ���
    private:
        int mRef;
        int mInterval;
        int mInvokeCount;
        int mMaxInvokeCount;
        lua_State *mLuaEnv;
    };
}// namespace Sentry