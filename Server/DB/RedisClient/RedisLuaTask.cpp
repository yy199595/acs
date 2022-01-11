#include "RedisLuaTask.h"

#include <Core/App.h>
#include <Util/Guid.h>
#include "Component/RedisComponent.h"
#include <Coroutine/TaskComponent.h>

namespace GameKeeper
{
    RedisLuaTask::RedisLuaTask(const std::string &cmd, lua_State *lua, int ref)
        : RedisTaskSource(cmd)
    {
        this->mLuaEnv = lua;
        this->mCoroutineRef = ref;
    }

    RedisLuaTask::~RedisLuaTask()
    {
        luaL_unref(this->mLuaEnv, LUA_REGISTRYINDEX, this->mCoroutineRef);
    }

    void RedisLuaTask::RunFinish()
    {
        lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->mCoroutineRef);
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
                    LOG_ERROR("[lua error] {0}", lua_tostring(mLuaEnv, -1));
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
        auto redisManager = App::Get().GetComponent<RedisComponent>();
        if (redisManager == nullptr)
        {
            return nullptr;
        }
        return new RedisLuaTask(cmd, lua, ref);
    }
}// namespace GameKeeper