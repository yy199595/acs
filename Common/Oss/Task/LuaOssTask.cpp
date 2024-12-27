//
// Created by yy on 2024/10/9.
//

#include "LuaOssTask.h"

namespace acs
{
	LuaOssRequestTask::LuaOssRequestTask(lua_State* lua, std::string url)
		:IRpcTask<http::Response>(0), mLua(lua), mRef(0), mUrl(std::move(url))
	{
		if(lua_isthread(this->mLua, -1))
		{
			this->mRef = luaL_ref(lua, LUA_REGISTRYINDEX);
		}
	}

	LuaOssRequestTask::~LuaOssRequestTask() noexcept
	{
		if(this->mRef != 0)
		{
			luaL_unref(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		}
	}

	int LuaOssRequestTask::Await()
	{
		if(this->mRef == 0)
		{
			luaL_error(this->mLua, "not lua coroutine context yield failure");
			return 0;
		}
		return lua_yield(this->mLua, 0);
	}

	void LuaOssRequestTask::OnResponse(std::unique_ptr<http::Response > response)
	{
		int count = 0;
		if(response != nullptr && response->Code() == HttpStatus::OK)
		{
			count++;
			lua_pushlstring(this->mLua, this->mUrl.c_str(), this->mUrl.size());
		}
		Lua::Coroutine::Resume(this->mLua, count);
	}
}