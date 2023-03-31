//
// Created by zmhy0073 on 2022/11/2.
//

#include"HttpTask.h"
#include"Util/Json/Lua/Json.h"
namespace Sentry
{
    HttpRequestTask::HttpRequestTask(int id)
        : IRpcTask<Http::Response>(id)
    {

    }

    void HttpRequestTask::OnTimeout()
    {
        std::shared_ptr<Http::Response> response(new Http::Response());
        {
            this->OnResponse(response);
        }
    }

    void HttpRequestTask::OnResponse(std::shared_ptr<Http::Response> response)
    {
        this->mTask.SetResult(response);
    }
}

namespace Sentry
{
    LuaHttpRequestTask::LuaHttpRequestTask(lua_State *lua)
        : IRpcTask<Http::Response>(0),mRef(0), mLua(lua)
    {
        //lua_pushthread(lua);
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

    void LuaHttpRequestTask::OnResponse(std::shared_ptr<Http::Response> response)
    {
        lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
        lua_State* coroutine = lua_tothread(this->mLua, -1);
        if(response != nullptr)
        {
            //TODO
            // response->GetData().Writer(this->mLua);
            int code = (int)response->Code();
            lua_pushinteger(this->mLua, code);
            const std::string & conetnt = response->GetContent();
            lua_pushlstring(this->mLua, conetnt.c_str(), conetnt.size());
            Lua::Coroutine::Resume(coroutine, this->mLua, 2);
            return;
        }
        Lua::Coroutine::Resume(coroutine, this->mLua, 0);
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