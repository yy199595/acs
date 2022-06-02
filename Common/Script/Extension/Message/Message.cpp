//
// Created by mac on 2022/6/1.
//

#include"Message.h"
#include"Pool/MessagePool.h"
#include"Script/UserDataParameter.h"
#include"Script/Extension/Json/Encoder.h"
#include"Component/Scene/MessageComponent.h"
using namespace Sentry;
namespace Lua
{
	int MessageEx::New(lua_State* lua)
	{
        MessageComponent * messageComponent = UserDataParameter::Read<MessageComponent*>(lua, 1);
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
		std::shared_ptr<Message> message = messageComponent->New(name, std::string(json, size));
		if (message == nullptr)
		{
			luaL_error(lua, "create %s message error", name);
			return 0;
		}
		UserDataParameter::Write(lua, message);
		return 1;
	}
}