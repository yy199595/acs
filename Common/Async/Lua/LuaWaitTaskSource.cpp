//
// Created by mac on 2022/5/31.
//
#include"LuaWaitTaskSource.h"
#include"XCode/XCode.h"
#include"Entity/Actor/App.h"
#include"Async/Lua/LuaCoroutine.h"
#include"Yyjson/Lua/ljson.h"
#include"Proto/Component/ProtoComponent.h"
#include<google/protobuf/util/json_util.h>
namespace acs
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

	void LuaWaitTaskSource::SetResult(int code, rpc::Packet * response)
	{
		int count = 1;
		lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		lua_State* coroutine = lua_tothread(this->mLua, -1);
		lua_pushinteger(this->mLua, code);
		if(code == XCode::Ok)
		{
			switch (response->GetProto())
			{
				case rpc::Porto::Json:
				{
					++count;
					const std::string& json = response->GetBody();
					lua::yyjson::write(this->mLua, json.c_str(), json.size());
				}
					break;
				case rpc::Porto::Protobuf:
				{
					std::string name;
					if(response->TempHead().Del("res", name))
					{
						pb::Message * message1 = App::GetProto()->Temp(name);
						if (message1 != nullptr)
						{
							++count;
							response->ParseMessage(message1);
							App::GetProto()->Write(this->mLua, *message1);
						}
					}
				}
					break;
				case rpc::Porto::String:
					++count;
					const std::string& str = response->GetBody();
					lua_pushlstring(this->mLua, str.c_str(), str.size());
					break;
			}
		}
		Lua::Coroutine::Resume(coroutine, this->mLua, count);
	}
}