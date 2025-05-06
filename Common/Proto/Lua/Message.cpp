//
// Created by mac on 2022/6/1.
//

#include "Message.h"
#include "Entity/Actor/App.h"
#include "Lua/Engine/UserDataParameter.h"


using namespace acs;

namespace lua
{
	int MessageEx::New(lua_State* lua)
	{
		const char* name = luaL_checkstring(lua, 1);
		ProtoComponent* messageComponent = App::GetProto();
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
			Lua::UserDataParameter::Write(lua,  message.release());
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
			Lua::UserDataParameter::Write(lua, message.release());
			return 1;
		}
		std::unique_ptr<pb::Message> message;
		if(!messageComponent->New(name, message))
		{
			return 0;
		}
		Lua::UserDataParameter::Write(lua,  message.release());
		return 1;
	}

	int MessageEx::ToJson(lua_State* lua)
	{
		if(lua_isuserdata(lua, 1))
		{
			pb::Message * pbMessage = Lua::UserDataParameter::Read<pb::Message*>(lua, 1);
			if(pbMessage == nullptr)
			{
				return 0;
			}
			std::string json;
			if(pb_json::MessageToJsonString(*pbMessage, &json).ok())
			{
				lua_pushlstring(lua, json.c_str(), json.size());
				return 1;
			}
		}
		return 0;
	}

	int MessageEx::Encode(lua_State* lua)
	{
		pb::Message* message = nullptr;
		if (lua_isuserdata(lua, 1))
		{
			message = Lua::UserDataParameter::Read<pb::Message*>(lua, 1);
		}
		else if (lua_istable(lua, 2))
		{
			const char* name = luaL_checkstring(lua, 1);
			message = App::GetProto()->Read(lua, name, 2);
		}
		else if (lua_isstring(lua, 2))
		{
			size_t size = 0;
			const char* name = luaL_checkstring(lua, 1);
			message = App::GetProto()->Temp(name);
			if (message == nullptr)
			{
				luaL_error(lua, "not find pb:%s", name);
				return 0;
			}
			const char* json = luaL_checklstring(lua, 2, &size);
			pb::StringPiece stringPiece(json, (int)size);
			if (!pb_json::JsonStringToMessage(stringPiece, message).ok())
			{
				luaL_error(lua, "json to pb:%s", name);
				return 0;
			}
		}
		if (message == nullptr)
		{
			return 0;
		}
		std::string data;
		message->SerializeToString(&data);
		lua_pushlstring(lua, data.c_str(), data.size());
		return 1;
	}

	int MessageEx::Decode(lua_State* lua)
	{
		size_t size = 0;
		const char * name = luaL_checkstring(lua, 1);
		const char * data = luaL_checklstring(lua, 2, &size);
		pb::Message * pbMessage = App::GetProto()->Temp(name);
		if(pbMessage == nullptr)
		{
			return 0;
		}

		if(!pbMessage->ParseFromArray(data, (int)size))
		{
			return 0;
		}
		App::GetProto()->Write(lua, *pbMessage);
		return 1;
	}

	int MessageEx::Import(lua_State* lua)
	{
		const char* name = luaL_checkstring(lua, 1);
		ProtoComponent* messageComponent = App::GetProto();
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
		int index = 0;
		lua_newtable(lua);
		for(const std::string & type : types)
		{
			index++;
			lua_pushinteger(lua, index);
			lua_pushstring(lua, type.c_str());
			lua_settable(lua, -3);
		}
		return 1;
	}
}