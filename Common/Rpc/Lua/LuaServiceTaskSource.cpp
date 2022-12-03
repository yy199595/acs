//
// Created by zmhy0073 on 2022/1/8.
//

#include"LuaServiceTaskSource.h"
#include"Lua/UserDataParameter.h"
#include"Lua/Message.h"
#include"Json/Lua/Json.h"
namespace Sentry
{
	LuaServiceTaskSource::LuaServiceTaskSource()
    {		
        this->mCode = XCode::LuaCoroutineWait;
    }

    XCode LuaServiceTaskSource::Await(std::shared_ptr<Message> message)
    {
		this->mRpcData = message;
        if(this->mCode == XCode::LuaCoroutineWait) 
		{
            this->mTaskSource.Await();
        }
        return this->mCode;
    }

	int LuaServiceTaskSource::SetRpc(lua_State* lua)
	{
		LuaServiceTaskSource* luaServiceTaskSource =
			Lua::UserDataParameter::Read<LuaServiceTaskSource*>(lua, 1);
		luaServiceTaskSource->mCode = (XCode)luaL_checkinteger(lua, 2);
		if (luaServiceTaskSource->mCode == XCode::Successful)
		{
			if (lua_istable(lua, 3) && luaServiceTaskSource->mRpcData != nullptr)
			{
				MessageEncoder messageEncoder(lua);
				if (!messageEncoder.Encode(luaServiceTaskSource->mRpcData, 3))
				{
					luaServiceTaskSource->mCode = XCode::ParseMessageError;
				}
			}
		}       
		luaServiceTaskSource->mTaskSource.SetResult();
		return 1;
	}
}

namespace Sentry
{
	XCode LuaServiceTaskSource::Await(Http::Response* message)
	{
		this->mHttpData = message;
		if (this->mCode == XCode::LuaCoroutineWait) 
		{
			this->mTaskSource.Await();
		}
		return this->mCode;
	}

	int LuaServiceTaskSource::SetHttp(lua_State* lua)
	{
		LuaServiceTaskSource* luaServiceTaskSource =
			Lua::UserDataParameter::Read<LuaServiceTaskSource*>(lua, 1);
		luaServiceTaskSource->mCode = (XCode)luaL_checkinteger(lua, 2);
		if (luaServiceTaskSource->mHttpData != nullptr)
		{
			if (lua_istable(lua, -1))
			{
				std::string data;
				Lua::Json::Read(lua, -1, &data);
				luaServiceTaskSource->mHttpData->Json(HttpStatus::OK, data);
			}
			else if (lua_isstring(lua, -1))
			{
				size_t size = 0;
				const char * json = luaL_checklstring(lua, -1, &size);
				luaServiceTaskSource->mHttpData->Json(HttpStatus::OK, json, size);
			}
		}
		luaServiceTaskSource->mTaskSource.SetResult();
		return 1;
	}
}