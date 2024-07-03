//
// Created by zmhy0073 on 2022/11/2.
//

#include"HttpTask.h"

namespace joke
{
    LuaHttpRequestTask::LuaHttpRequestTask(lua_State *lua)
        : IRpcTask<http::Response>(0), mRef(0), mLua(lua)
    {
        if(lua_isthread(this->mLua, -1))
        {
            this->mRef = luaL_ref(lua, LUA_REGISTRYINDEX);
        }
    }

    LuaHttpRequestTask::~LuaHttpRequestTask()
    {
        if(this->mRef != 0)
        {
            luaL_unref(this->mLua, LUA_REGISTRYINDEX, this->mRef);
        }
    }

    void LuaHttpRequestTask::OnResponse(http::Response * response)
    {
        int count = 0;
        lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
        lua_State* coroutine = lua_tothread(this->mLua, -1);
        if(response != nullptr)
        {
            count = response->WriteToLua(this->mLua);       
        }
        Lua::Coroutine::Resume(coroutine, this->mLua, count);
		delete response;
    }

    int LuaHttpRequestTask::Await()
    {
        if(this->mRef == 0)
        {
            luaL_error(this->mLua, "not lua coroutine context yield failure");
            return 0;
        }
        return lua_yield(this->mLua, 0);
    }
}