//
// Created by mac on 2022/6/1.
//

#include"Message.h"
#include"App/App.h"
#include"Script/UserDataParameter.h"
#include"Script/Extension/Json/Encoder.h"
#include<google/protobuf/util/json_util.h>
using namespace Sentry;

namespace Lua
{
	int MessageEx::New(lua_State* lua)
	{
        ProtocolComponent * messageComponent = UserDataParameter::Read<ProtocolComponent*>(lua, 1);
		if (!lua_isstring(lua, 2))
		{
			luaL_error(lua, "first parameter must string");
			return 0;
		}
		const char* name = lua_tostring(lua, 2);
		if (lua_istable(lua, 3))
		{
			UserDataParameter::Write(lua,  messageComponent->Read(lua, name, 3));
			return 1;
		}
		UserDataParameter::Write(lua,  messageComponent->New(name));
		return 1;
	}

	int MessageEx::NewJson(lua_State* lua)
	{
		ProtocolComponent* messageComponent = UserDataParameter::Read<ProtocolComponent*>(lua, 1);
		if (!lua_isstring(lua, 2))
		{
			luaL_error(lua, "first parameter must string");
			return 0;
		}
		const char* name = lua_tostring(lua, 2);
		if (!lua_istable(lua, 3))
		{
			luaL_error(lua, "second parameter must table");
			return 0;
		}
		StringBuffer s;
		Encoder encode(lua, 3);
		encode.encode(lua, &s, 3);
		const char* json = s.GetString();
		const size_t size = s.GetLength();
		UserDataParameter::Write(lua, messageComponent->New(name, std::string(json, size)));
		return 1;
	}

	int MessageEx::Encode(lua_State* lua)
	{
		return 0;
	}

	int MessageEx::Decode(lua_State* lua)
	{
		ProtocolComponent* messageComponent = UserDataParameter::Read<ProtocolComponent*>(lua, 1);
		std::shared_ptr<Message> message = UserDataParameter::Read<std::shared_ptr<Message>>(lua, 2);
		return messageComponent->Write(lua, *message) ? 1 : 0;
	}
}