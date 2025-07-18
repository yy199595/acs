//
// Created by zmhy0073 on 2022/1/8.
//

#include"LuaServiceTaskSource.h"
#include"Lua/Engine/UserDataParameter.h"
#include"Proto/Lua/Message.h"
#include"XCode/XCode.h"
#include"Entity/Actor/App.h"
#include"Yyjson/Lua/ljson.h"
namespace acs
{
	LuaServiceTaskSource::LuaServiceTaskSource(http::Response* message)
		: mHttpData(message), mRpcData(nullptr)
    {		
        this->mCode = XCode::LuaCoroutineWait;
    }

	LuaServiceTaskSource::LuaServiceTaskSource(rpc::Message* packet)
		: mHttpData(nullptr), mRpcData(packet)
	{
		this->mCode = XCode::LuaCoroutineWait;
	}

    int LuaServiceTaskSource::Await() noexcept
    {
        if(this->mCode == XCode::LuaCoroutineWait) 
		{
            this->mTaskSource.Await();
        }
        return this->mCode;
    }

	void LuaServiceTaskSource::WriteRpcResponse(lua_State* lua) noexcept
	{
		this->mRpcData->Body()->clear();
		this->mRpcData->SetProto(rpc::proto::none);
		this->mCode = (int)luaL_checkinteger(lua, 2);
		if (this->mCode != XCode::Ok)
		{
			return;
		}
		if (lua_istable(lua, 3))
		{
			std::string pb;
			if (this->mRpcData->TempHead().Get("pb", pb))
			{
				this->mRpcData->SetProto(rpc::proto::pb);
				pb::Message* message = App::GetProto()->Temp(pb);
				if (message == nullptr)
				{
					this->mCode = XCode::CreateProtoFailure;
					return;
				}
				MessageEncoder messageEncoder(lua);
				if (!messageEncoder.Encode(*message, 3))
				{
					this->mCode = XCode::ParseMessageError;
					return;
				}
				message->SerializePartialToString(this->mRpcData->Body());
			}
			else
			{
				this->mRpcData->SetProto(rpc::proto::json);
				lua::yyjson::read(lua, 3, *this->mRpcData->Body());
			}
		}
		else if (lua_isstring(lua, 3))
		{
			size_t len = 0;
			const char* str = lua_tolstring(lua, 3, &len);
			this->mRpcData->Body()->append(str, len);
			this->mRpcData->SetProto(rpc::proto::string);
		}
	}

	int LuaServiceTaskSource::SetRpc(lua_State* lua) noexcept
	{
		LuaServiceTaskSource* luaServiceTaskSource =
			Lua::UserDataParameter::Read<LuaServiceTaskSource*>(lua, 1);

		luaServiceTaskSource->WriteRpcResponse(lua);
		luaServiceTaskSource->mTaskSource.SetResult();
		return 1;
	}
}

namespace acs
{
	int LuaServiceTaskSource::SetHttp(lua_State* lua) noexcept
	{
		LuaServiceTaskSource* luaServiceTaskSource =
			Lua::UserDataParameter::Read<LuaServiceTaskSource*>(lua, 1);
		if (luaServiceTaskSource->mHttpData != nullptr)
		{
			luaServiceTaskSource->mCode = (int)luaL_checkinteger(lua, 2);
			if(luaServiceTaskSource->mCode == XCode::Ok)
			{
				if (lua_istable(lua, 3))
				{
					lua::JsonValue jsonValue(true);
					lua::yyjson::read(lua, -1, jsonValue);
					{
						json::w::Document document;
						document.Add("code", XCode::Ok);
						document.Add("data", jsonValue.val);
						luaServiceTaskSource->mHttpData->Json(document);
					}
				}
				else if (lua_isstring(lua, 3))
				{
					size_t size = 0;
					const char* str = luaL_checklstring(lua, -1, &size);
					luaServiceTaskSource->mHttpData->Text(str, size);
				}
			}
		}
		luaServiceTaskSource->mTaskSource.SetResult();
		return 1;
	}
}