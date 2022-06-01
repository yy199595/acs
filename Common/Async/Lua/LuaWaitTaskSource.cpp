//
// Created by mac on 2022/5/31.
//

#include"LuaWaitTaskSource.h"
#include"Pool/MessagePool.h"
#include"Script/Extension/Json/values.hpp"
namespace Sentry
{
	LuaWaitTaskSource::LuaWaitTaskSource(lua_State* lua)
			: mLua(lua), mRef(0)
	{
		if(lua_isthread(this->mLua, -1))
		{
			this->mRef = luaL_ref(lua, LUA_REGISTRYINDEX);
		}
	}
	LuaWaitTaskSource::~LuaWaitTaskSource()
	{
		if(this->mRef != 0)
		{
			luaL_unref(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		}
	}

	int LuaWaitTaskSource::Await()
	{
		if(this->mRef == 0)
		{
			luaL_error(this->mLua, "not lua coroutine context yield failure");
			return 0;
		}
		return lua_yield(this->mLua, 0);
	}

	void LuaWaitTaskSource::SetResult(XCode code, std::shared_ptr<Message> response)
	{
		lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		lua_State* coroutine = lua_tothread(this->mLua, -1);
		lua_pushinteger(this->mLua, (int)code);
		if (code == XCode::Successful && response != nullptr)
		{
			std::string json;
			if (Proto::GetJson(response, json))
			{
				values::pushDecoded(this->mLua, json.c_str(), json.size());
				lua_presume(coroutine, this->mLua, 2);
				return;
			}
		}
		lua_presume(coroutine, this->mLua, 1);
	}
	void LuaWaitTaskSource::SetResult()
	{
		lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		lua_presume(lua_tothread(this->mLua, -1), this->mLua, 0);
	}
	void LuaWaitTaskSource::SetJson(const string& json)
	{
		lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		lua_State* coroutine = lua_tothread(this->mLua, -1);
		values::pushDecoded(this->mLua, json.c_str(), json.size());
		lua_presume(coroutine, this->mLua, 1);

	}
}