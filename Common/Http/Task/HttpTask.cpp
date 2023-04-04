//
// Created by zmhy0073 on 2022/11/2.
//

#include"HttpTask.h"
#include"Util/Json/Lua/Json.h"
namespace Sentry
{
    HttpRequestTask::HttpRequestTask()
        : IRpcTask<Http::IResponse>(0)
    {

    }

    void HttpRequestTask::OnResponse(std::shared_ptr<Http::IResponse> response)
    {
        this->mTask.SetResult(response);
    }
}

namespace Sentry
{
    LuaHttpRequestTask::LuaHttpRequestTask(lua_State *lua)
        : IRpcTask<Http::IResponse>(0), mRef(0), mLua(lua)
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

    void LuaHttpRequestTask::OnResponse(std::shared_ptr<Http::IResponse> response)
    {
        lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
        lua_State* coroutine = lua_tothread(this->mLua, -1);
        if(response != nullptr)
        {
            int count = 1;
            int code = (int)response->Code();
            lua_pushinteger(this->mLua, code);
            if(std::shared_ptr<Http::DataResponse> dataResponse =
                std::dynamic_pointer_cast<Http::DataResponse>(response))
            {
                count++;
                const std::string & content = dataResponse->GetContent();
                lua_pushlstring(this->mLua, content.c_str(), content.size());
            }
            Lua::Coroutine::Resume(coroutine, this->mLua, count);
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