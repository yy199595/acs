//
// Created by mac on 2022/6/1.
//

#include"Message.h"
#include"Entity/App/App.h"
#include"Util/Json/Lua/Encoder.h"
#include"Script/Lua/UserDataParameter.h"
#include"google/protobuf/util/json_util.h"
using namespace Sentry;

namespace Lua
{
	int MessageEx::New(lua_State* lua)
	{
		const char* name = luaL_checkstring(lua, 1);
		ProtoComponent* messageComponent = App::Inst()->GetComponent<ProtoComponent>();
		if (messageComponent == nullptr)
		{
			luaL_error(lua, "ProtoComponent Is Null");
			return 0;
		}
		if (lua_istable(lua, 2))
		{
			UserDataParameter::Write(lua,  messageComponent->Read(lua, name, 2));
			return 1;
		}
		else if (lua_isstring(lua, 2))
		{
			size_t size = 0;
			const char * json = luaL_checklstring(lua, 2, &size);
			UserDataParameter::Write(lua, messageComponent->New(name, json, size));
			return 1;
		}
		UserDataParameter::Write(lua,  messageComponent->New(name));
		return 1;
	}

	int MessageEx::Import(lua_State* lua)
	{
		const char* name = luaL_checkstring(lua, 1);
		ProtoComponent* messageComponent = App::Inst()->GetComponent<ProtoComponent>();
		if (messageComponent == nullptr)
		{
			luaL_error(lua, "ProtoComponent Is Null");
			return 0;
		}
		lua_pushboolean(lua, messageComponent->Import(name));
		return 1;
	}
}