#include "RedisLuaTask.h"

#include <Core/App.h>
#include <Util/NumberHelper.h>
#include <Scene/RedisComponent.h>
#include <Coroutine/CoroutineComponent.h>

namespace Sentry
{
    RedisLuaTask::RedisLuaTask(const std::string &cmd, lua_State *lua, int ref)
        : RedisTaskBase(cmd)
    {
        this->mLuaEnv = lua;
        this->mCoroutienRef = ref;
    }

    RedisLuaTask::~RedisLuaTask()
    {
        luaL_unref(this->mLuaEnv, LUA_REGISTRYINDEX, this->mCoroutienRef);
    }

    void RedisLuaTask::RunFinish()
    {
        lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->mCoroutienRef);
        if (lua_isthread(this->mLuaEnv, -1))
        {
            lua_State *coroutine = lua_tothread(this->mLuaEnv, -1);
            if (lua_getfunction(this->mLuaEnv, "JsonUtil", "ToObject"))
            {
                const char *json = this->mQueryJsonData.c_str();
                const size_t size = this->mQueryJsonData.size();
                lua_pushlstring(this->mLuaEnv, json, size);
                if (lua_pcall(this->mLuaEnv, 1, 1, 0) != 0)
                {
                    SayNoDebugError("[lua error] " << lua_tostring(mLuaEnv, -1));
                }
            }
			lua_presume(coroutine, this->mLuaEnv, 1);
        }
    }

    RedisLuaTask * RedisLuaTask::Create(lua_State *lua, int index, const char *cmd)
    {
        if (!lua_isthread(lua, index))
        {
            return nullptr;
        }       
        lua_State *coroutine = lua_tothread(lua, index);
        int ref = luaL_ref(lua, LUA_REGISTRYINDEX);
        RedisComponent *redisManager = Scene::GetComponent<RedisComponent>();
        if (redisManager == nullptr)
        {
            return nullptr;
        }
        return new RedisLuaTask(cmd, lua, ref);
    }
}// namespace Sentry