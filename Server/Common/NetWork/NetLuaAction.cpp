#include"NetLuaAction.h"
#include<Other/ObjectFactory.h>

namespace SoEasy
{
	NetLuaAction * NetLuaAction::Create(lua_State * luaEvn, std::string table, std::string func)
	{
		lua_getglobal(luaEvn, table.c_str());
		if (lua_istable(luaEvn, -1))
		{
			lua_getfield(luaEvn, -1, func.c_str());
			if (lua_isfunction(luaEvn, -1))
			{
				int ref = luaL_ref(luaEvn, LUA_REGISTRYINDEX);
				std::string name = table + "." + func;
				return new NetLuaAction(luaEvn, ref, name);
			}
		}
		return nullptr;
	}

	XCode NetLuaAction::Invoke1(shared_ptr<TcpClientSession> session, long long operatorId)
	{
		lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->ref);
		if (lua_isfunction(this->luaEnv, -1))
		{
			LuaParameter::Write<shared_ptr<TcpClientSession>>(this->luaEnv, session);
			lua_pushinteger(this->luaEnv, operatorId);
			if (lua_pcall(this->luaEnv, 2, 1, 0) != 0)
			{
				const char * error = lua_tostring(luaEnv, -1);
				SayNoDebugError("call lua function error " << error);
				return XCode::CallLuaFunctionFail;
			}
			return (XCode)lua_tointeger(this->luaEnv, -1);
		}
		return XCode::CallLuaFunctionFail;
	}

	XCode NetLuaAction::Invoke(shared_ptr<TcpClientSession> tcpSession, const shared_ptr<NetWorkPacket> requestData, shared_ptr<NetWorkPacket> returnData)
	{
		lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->ref);
		if (!lua_isfunction(this->luaEnv, -1))
		{
			return XCode::CallLuaFunctionFail;
		}
		int argsCount = 2;
		const long long operatorId = requestData->operator_id();
		const std::string & name = requestData->protoc_name();
		const std::string & message = requestData->message_data();
		LuaParameter::Write<shared_ptr<TcpClientSession>>(this->luaEnv, tcpSession);
		lua_pushinteger(this->luaEnv, operatorId);
		if (!message.empty() && lua_getfunction(this->luaEnv, "JsonUtil", "ToObject"))
		{
			Message * pMessage = ObjectFactory::Get()->CreateMessage(name);
			if (pMessage != nullptr && pMessage->ParseFromString(message))
			{
				std::string jsonString;
				ProtocHelper::GetJsonString(pMessage, jsonString);
				lua_pushlstring(this->luaEnv, jsonString.c_str(), jsonString.size());
			}
			else
			{
				lua_pushlstring(this->luaEnv, message.c_str(), message.size());
			}
			if (lua_pcall(this->luaEnv, 1, 1, 0) != 0)
			{
				SayNoDebugError(lua_tostring(this->luaEnv, -1));
				return XCode::CallLuaFunctionFail;
			}
			argsCount++;
		}
	
		if (lua_pcall(this->luaEnv, argsCount, 2, 0) != 0)
		{
			SayNoDebugError("call lua function error : " << lua_tostring(this->luaEnv, -1));
			return XCode::CallLuaFunctionFail;
		}
		XCode code = (XCode)lua_tointeger(this->luaEnv, -2);
		if (lua_isuserdata(this->luaEnv, -1))
		{
			std::string messageBuffer;
			Message * pRetMessage = (Message *)lua_touserdata(this->luaEnv, -1);
			if (pRetMessage != nullptr && pRetMessage->SerializePartialToString(&messageBuffer))
			{
				returnData->set_message_data(messageBuffer);
				returnData->set_protoc_name(pRetMessage->GetTypeName());
			}
		}
		else if (lua_istable(this->luaEnv, -1) && lua_getfunction(this->luaEnv, "JsonUtil", "ToString"))
		{
			lua_pushvalue(this->luaEnv, -2);
			if (lua_pcall(this->luaEnv, 1, 1, 0) != 0)
			{
				SayNoDebugError("call lua function error : " << lua_tostring(this->luaEnv, -1));
				return XCode::CallLuaFunctionFail;
			}
			size_t size = 0;
			const char * json = lua_tolstring(this->luaEnv, -1, &size);
			returnData->set_message_data(json, size);
		}
		return code;
	}
}