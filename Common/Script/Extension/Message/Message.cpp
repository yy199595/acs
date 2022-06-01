//
// Created by mac on 2022/6/1.
//

#include"Message.h"
#include"Pool/MessagePool.h"
#include"Script/UserDataParameter.h"
#include"Script/Extension/Json/Encoder.h"
namespace Lua
{
	int MessageEx::New(lua_State* lua)
	{
		if (!lua_isstring(lua, 1))
		{
			luaL_error(lua, "first parameter must string");
			return 0;
		}
		const char* name = lua_tostring(lua, 1);
		if (!lua_istable(lua, 2))
		{
			luaL_error(lua, "second parameter must table");
			return 0;
		}

		StringBuffer s;
		Encoder encode(lua, 2);
		encode.encode(lua, &s, 2);
		const char* json = s.GetString();
		const size_t size = s.GetLength();
		std::shared_ptr<Message> message = Helper::Proto::NewByJson(name, json, size);
		if (message == nullptr)
		{
			luaL_error(lua, "create %s message error", name);
			return 0;
		}
		UserDataParameter::Write(lua, message);
		return 1;
	}
}