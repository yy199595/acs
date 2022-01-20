#pragma once

#include"RedisDefine.h"
#include"RedisTaskSource.h"
#include<Script/LuaInclude.h>

#define RedisLuaArgvSize 10
namespace Sentry
{
    class QuertJsonWritre;

    class RedisLuaTask : public RedisTaskSource
    {
    public:
        RedisLuaTask(const std::string &cmd, lua_State *lua, int ref);

        ~RedisLuaTask() final;

    protected:
        void RunFinish() final;  //执行完成之后在主线程调用
    public:
        static RedisLuaTask * Create(lua_State *lua, int index, const char *cmd);

    private:
        int mCoroutineRef;
        lua_State *mLuaEnv;
    private:
        std::string mQueryJsonData;
    };
}