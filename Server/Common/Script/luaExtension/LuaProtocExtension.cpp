#include "LuaProtocExtension.h"
#include<Define/CommonDef.h>

#include<google/protobuf/message.h>
#include<google/protobuf/descriptor.h>
#include<google/protobuf/util/json_util.h>
#include<Other/ObjectFactory.h>
using namespace google::protobuf;
using namespace google::protobuf::util;
using namespace SoEasy;
namespace LuaProtocExtension
{
	int Serialization(lua_State * lua)
	{
		SayNoAssertRetZero_F(lua_istable(lua, -1));
		SayNoAssertRetZero_F(lua_isstring(lua, -2));
		const char * name = lua_tostring(lua, -2);

		const Descriptor * desc = DescriptorPool::generated_pool()->FindMessageTypeByName(name);
		SayNoAssertRetVal(desc, "nod find protoc name : " << name, 0);
		SayNoAssertRetZero_F(lua_getfunction(lua, "JsonUtil", "Encode"));

		lua_pushvalue(lua, -3);
		if (lua_pcall(lua, 1, 1, 0) == 0)
		{
			size_t size = 0;
			const char * jsonString = lua_tolstring(lua, -1, &size);
			Message * message = MessageFactory::generated_factory()->GetPrototype(desc)->New();
			if (util::JsonStringToMessage(std::string(jsonString, size), message).ok())
			{
				std::string data = message->SerializeAsString();
				lua_pushlstring(lua, data.c_str(), data.size());
			}
			delete message;
		}
		return 1;
	}

	bool Parse(const std::string json, Message * message)
	{
		return JsonStringToMessage(json, message).ok();
	}

	int CreateByTable(lua_State * lua)
	{
		if (!lua_isstring(lua, 1) || !lua_istable(lua, 2))
		{
			lua_pushnil(lua);
			return 1;
		}
		const char * name = lua_tostring(lua, 1);
		Message * pMessage = ObjectFactory::Get()->CreateMessage(name);
		if (pMessage == nullptr || !lua_getfunction(lua, "JsonUtil", "ToString"))
		{
			lua_pushnil(lua);
			return 1;
		}
		lua_pushvalue(lua, 2);
		if (lua_pcall(lua, 1, 1, 0) != 0)
		{
			SayNoDebugError(lua_tostring(lua, -1));
			lua_pushnil(lua);
			return 1;
		}
		size_t size = 0;
		const char * json = lua_tolstring(lua, -1, &size);
		if (JsonStringToMessage(std::string(json, size), pMessage).ok())
		{
			lua_pushlightuserdata(lua, pMessage);
			return 1;
		}
		return 0;
	}
}

