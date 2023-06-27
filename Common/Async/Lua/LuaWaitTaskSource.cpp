//
// Created by mac on 2022/5/31.
//
#include"LuaWaitTaskSource.h"
#include"XCode/XCode.h"
#include"Entity/Actor/App.h"
#include"Async/Lua/LuaCoroutine.h"
#include"Proto/Component/ProtoComponent.h"
#include<google/protobuf/util/json_util.h>
namespace Tendo
{
	LuaWaitTaskSource::LuaWaitTaskSource(lua_State* lua)
			: mRef(0), mLua(lua)
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

	void LuaWaitTaskSource::SetResult()
	{
		lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
        Lua::Coroutine::Resume(lua_tothread(this->mLua, -1), this->mLua, 0);
	}
	void LuaWaitTaskSource::SetResult(int code, const std::shared_ptr<pb::Message>& response)
	{
		lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		lua_State* coroutine = lua_tothread(this->mLua, -1);
		lua_pushinteger(this->mLua, (int)code);
		if (code == XCode::Successful && response != nullptr)
		{
			if(!App::Inst()->GetProto()->Write(this->mLua, *response))
			{
				LOG_ERROR("write response message to lua fail : " << response->GetTypeName());
			}
			Lua::Coroutine::Resume(coroutine, this->mLua, 2);
			return;
		}
		Lua::Coroutine::Resume(coroutine, this->mLua, 1);
	}
}