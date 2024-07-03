//
// Created by mac on 2022/6/1.
//

#include"Message.h"
#include"Entity/Actor/App.h"
#include"Lua/Engine/UserDataParameter.h"

using namespace joke;

namespace Lua
{
	int MessageEx::New(lua_State* lua)
	{
		const char* name = luaL_checkstring(lua, 1);
		ProtoComponent* messageComponent = App::Inst()->GetProto();
		if (messageComponent == nullptr)
		{
			luaL_error(lua, "ProtoComponent Is Null");
			return 0;
		}
		if (lua_istable(lua, 2))
		{
			std::unique_ptr<pb::Message> message;
			pb::Message * data = messageComponent->Read(lua, name, 2);
			if(data == nullptr)
			{
				return 0;
			}
			message.reset(data->New());
			UserDataParameter::Write(lua,  message.release());
			return 1;
		}
		else if (lua_isstring(lua, 2))
		{
			size_t size = 0;
			std::unique_ptr<pb::Message> message;
			const char * json = luaL_checklstring(lua, 2, &size);
			if(!messageComponent->New(name, json, size, message))
			{
				return 0;
			}
			UserDataParameter::Write(lua, message.release());
			return 1;
		}
		std::unique_ptr<pb::Message> message;
		if(!messageComponent->New(name, message))
		{
			return 0;
		}
		UserDataParameter::Write(lua,  message.release());
		return 1;
	}

	int MessageEx::Encode(lua_State* lua)
	{
		std::unique_ptr<pb::Message> message;
		const char* name = luaL_checkstring(lua, 1);
		ProtoComponent* messageComponent = App::Inst()->GetProto();
		if (lua_istable(lua, 2))
		{
			pb::Message * data = messageComponent->Read(lua, name, 2);
			if(data == nullptr)
			{
				return 0;
			}
			message.reset(data);
		}
		else if(lua_isstring(lua, 2))
		{
			size_t size = 0;
			const char * json = luaL_checklstring(lua, 2, &size);
			if(!messageComponent->New(name, json, size, message))
			{
				return 0;
			}
		}
		if(message != nullptr)
		{
			std::string data;
			message->SerializeToString(&data);
			lua_pushlstring(lua, data.c_str(), data.size());
			return 1;
		}
		return 0;
	}

	int MessageEx::Decode(lua_State* lua)
	{
		size_t size = 0;
		std::unique_ptr<pb::Message> message;
		const char * name = luaL_checkstring(lua, 1);
		const char * data = luaL_checklstring(lua, 2, &size);
		ProtoComponent* messageComponent = App::Inst()->GetProto();
		if(!messageComponent->New(name, message))
		{
			return 0;
		}
		if(!message->ParseFromArray(data, (int)size))
		{
			return 0;
		}
		messageComponent->Write(lua, *message);
		return 1;
	}

	int MessageEx::Import(lua_State* lua)
	{
		const char* name = luaL_checkstring(lua, 1);
		ProtoComponent* messageComponent = App::Inst()->GetProto();
		if (messageComponent == nullptr)
		{
			luaL_error(lua, "ProtoComponent Is Null");
			return 0;
		}
		std::vector<std::string> types;
		if(!messageComponent->Import(name, types))
		{
			return 0;
		}
		size_t index = 0;
		lua_newtable(lua);
		int top = lua_gettop(lua);
		for(const std::string & type : types)
		{
			index++;
			lua_pushinteger(lua, index);
			lua_pushstring(lua, type.c_str());
		}
		lua_settop(lua, top);
		return 1;
	}
}