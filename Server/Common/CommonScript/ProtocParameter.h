#pragma once
#include"LuaInclude.h"
#include"LuaDebugStack.h"
#include<google/protobuf/message.h>
#include<google/protobuf/util/json_util.h>
using namespace std;
using namespace google::protobuf;
using namespace google::protobuf::util;
namespace ProtocParameter
{
	template<typename T>
	struct ProtocStruct
	{
		static T Read(lua_State * lua, int index) 
		{
			if (lua_istable(lua, index) && lua_getfunction(lua, "JsonUtil", "ToString"))
			{
				T message;
				lua_pushvalue(lua, index - 1);
				if (lua_pcall(lua, 1, 1, 0) != 0)
				{
					printf("%s", lua_tostring(lua, -1));
					return nullptr;
				}
				size_t size = 0;
				const char * str = lua_tolstring(lua, -1, &size);
				if (!JsonStringToMessage(StringPiece(str, size), &message).ok())
				{
					printf("Decode json error : %s", str);
					return nullptr;
				}
				return message;
			}
			return T();
		}

		static void Write(lua_State * lua, T data) 
		{
			if (lua_getfunction(lua, "JsonUtil", "ToObject"))
			{
				std::string outString = "";
				if (MessageToJsonString(data, &outString).ok())
				{
					lua_pushlstring(lua, outString.c_str(), outString.size());
					if (lua_pcall(lua, 1, 1, 0) != 0)
					{
						const char * error = lua_tostring(lua, -1);
						printf("json to object error : %s", error);
					}
				}
			}
		}
	};
}

namespace ProtocParameter
{
	template<typename T>
	struct ProtocStruct<T *>
	{
		static T * Read(lua_State * lua, int index)
		{
			static T message;
			if (lua_istable(lua, index) && lua_getfunction(lua, "JsonUtil", "ToString"))
			{
				lua_pushvalue(lua, index - 1);
				if (lua_pcall(lua, 1, 1, 0) != 0)
				{
					printf("%s", lua_tostring(lua, -1));
					return nullptr;
				}
				size_t size = 0;
				const char * str = lua_tolstring(lua, -1, &size);
				if (!JsonStringToMessage(StringPiece(str, size), &message).ok())
				{
					printf("Decode json error : %s", str);
					return nullptr;
				}
				return &message;
			}
			return nullptr;
		}

		static void Write(lua_State * lua, T * data)
		{		
			if (lua_getfunction(lua, "JsonUtil", "ToObject"))
			{
				std::string outString = "";
				if (MessageToJsonString(*data, &outString).ok())
				{			
					lua_pushlstring(lua, outString.c_str(), outString.size());
					if (lua_pcall(lua, 1, 1, 0) != 0)
					{
						const char * error = lua_tostring(lua, -1);
						printf("json to object error : %s", error);
					}
				}
			}		
		}
	};

	template<typename T>
	struct ProtocStruct<T &>
	{
		static T & Read(lua_State * lua, int index)
		{
			static T message;
			message.Clear();
			if (lua_istable(lua, index) && lua_getfunction(lua, "JsonUtil", "ToString"))
			{
				lua_pushvalue(lua, index - 1);
				if (lua_pcall(lua, 1, 1, 0) != 0)
				{
					printf("%s", lua_tostring(lua, -1));
					return nullptr;
				}
				size_t size = 0;
				const char * str = lua_tolstring(lua, -1, &size);
				if (!JsonStringToMessage(StringPiece(str, size), &message).ok())
				{
					printf("Decode json error : %s", str);
					return nullptr;
				}
				return message;
			}
			return nullptr;
		}
		static void Write(lua_State * lua, T & data)
		{
			if (lua_getfunction(lua, "JsonUtil", "ToObject"))
			{
				std::string outString = "";
				if (MessageToJsonString(data, &outString).ok())
				{
					lua_pushlstring(lua, outString.c_str(), outString.size());
					if (lua_pcall(lua, 1, 1, 0) != 0)
					{
						const char * error = lua_tostring(lua, -1);
						printf("json to object error : %s", error);
					}
				}
			}
		}
	};
}

namespace ProtocParameter
{
	template<typename T>
	inline T Read(lua_State * lua, int index)
	{
		return ProtocStruct<T>::Read(lua, index);
	}

	template<typename T>
	inline void Write(lua_State * lua, T data)
	{
		ProtocStruct<T>::Write(lua, data);
	}
}