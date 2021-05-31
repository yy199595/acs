#include"NetLuaAction.h"
#include<Other/ObjectFactory.h>

namespace SoEasy
{
	NetLuaAction::NetLuaAction(lua_State * lua, const std::string name, int action_ref)
	{
		this->luaEnv = lua;
		this->mActionName = name;
		this->mActionref = action_ref;
		SayNoAssertRet_F(lua_getfunction(lua, "ServiceProxy", "Invoke"));
		this->mLocalref = luaL_ref(this->luaEnv, LUA_REGISTRYINDEX);
		SayNoAssertRet_F(lua_getfunction(lua, "ServiceProxy", "InvokeAddress"));
		this->mProxyref = luaL_ref(this->luaEnv, LUA_REGISTRYINDEX);
	}

	NetLuaAction::~NetLuaAction()
	{
		luaL_unref(luaEnv, LUA_REGISTRYINDEX, this->mLocalref);
		luaL_unref(luaEnv, LUA_REGISTRYINDEX, this->mProxyref);
		luaL_unref(luaEnv, LUA_REGISTRYINDEX, this->mActionref);
	}

	XCode NetLuaAction::Invoke(const shared_ptr<NetWorkPacket> requestData)
	{
		lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->mLocalref);
		if (!lua_isfunction(this->luaEnv, -1))
		{
			return XCode::CallLuaFunctionFail;
		}

		lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->mActionref);
		if (!lua_isfunction(this->luaEnv, -1))
		{
			return XCode::CallLuaFunctionFail;
		}
		lua_pushinteger(luaEnv, requestData->operator_id());
		lua_pushinteger(luaEnv, requestData->callback_id());
		const std::string & protocolName = requestData->protoc_name();
		if (!protocolName.empty())
		{
			const std::string & message = requestData->message_data();
			Message * protocolMessage = ObjectFactory::Get()->CreateMessage(protocolName);
			if (protocolMessage != nullptr && protocolMessage->ParseFromString(message))
			{
				std::string jsonString;
				if (!ProtocHelper::GetJsonString(protocolMessage, jsonString))
				{
					return XCode::ProtocbufCastJsonFail;
				}
				lua_pushlstring(luaEnv, jsonString.c_str(), jsonString.size());
			}
		}
		else
		{
			const std::string & jsonString = requestData->message_data();
			lua_pushlstring(luaEnv, jsonString.c_str(), jsonString.size());
		}

		if (lua_pcall(luaEnv, 4, 0, 0) != 0)
		{
			SayNoDebugError(lua_tostring(luaEnv, -1));
			return XCode::CallLuaFunctionFail;
		}
		return XCode::LuaCoroutineReturn;
	}

	XCode NetLuaAction::Invoke(const std::string & address, const shared_ptr<NetWorkPacket> requestData)
	{
		lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->mProxyref);
		if (!lua_isfunction(this->luaEnv, -1))
		{
			return XCode::CallLuaFunctionFail;
		}

		lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->mActionref);
		if (!lua_isfunction(this->luaEnv, -1))
		{
			return XCode::CallLuaFunctionFail;
		}
		lua_pushstring(luaEnv, address.c_str());
		lua_pushinteger(luaEnv, requestData->operator_id());
		lua_pushinteger(luaEnv, requestData->callback_id());
		const std::string & protocolName = requestData->protoc_name();
		if (!protocolName.empty())
		{
			const std::string & message = requestData->message_data();
			Message * protocolMessage = ObjectFactory::Get()->CreateMessage(protocolName);
			if (protocolMessage != nullptr && protocolMessage->ParseFromString(message))
			{
				std::string jsonString;
				if (!ProtocHelper::GetJsonString(protocolMessage, jsonString))
				{
					return XCode::ProtocbufCastJsonFail;
				}
				lua_pushlstring(luaEnv, jsonString.c_str(), jsonString.size());
			}
		}
		else
		{
			const std::string & jsonString = requestData->message_data();
			lua_pushlstring(luaEnv, jsonString.c_str(), jsonString.size());
		}

		if (lua_pcall(luaEnv, 5, 0, 0) != 0)
		{
			SayNoDebugError(lua_tostring(luaEnv, -1));
			return XCode::CallLuaFunctionFail;
		}
		return XCode::LuaCoroutineReturn;
	}

	//XCode NetLuaAction::Invoke(shared_ptr<TcpClientSession> tcpSession, const shared_ptr<NetWorkPacket> requestData, shared_ptr<NetWorkPacket> returnData)
	//{
	//	lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->ref);
	//	if (!lua_isfunction(this->luaEnv, -1))
	//	{
	//		return XCode::CallLuaFunctionFail;
	//	}
	//	int argsCount = 2;
	//	const long long operatorId = requestData->operator_id();
	//	const std::string & name = requestData->protoc_name();
	//	const std::string & message = requestData->message_data();
	//	LuaParameter::Write<shared_ptr<TcpClientSession>>(this->luaEnv, tcpSession);
	//	lua_pushinteger(this->luaEnv, operatorId);
	//	if (!message.empty() && lua_getfunction(this->luaEnv, "JsonUtil", "ToObject"))
	//	{
	//		Message * pMessage = ObjectFactory::Get()->CreateMessage(name);
	//		if (pMessage != nullptr && pMessage->ParseFromString(message))
	//		{
	//			std::string jsonString;
	//			ProtocHelper::GetJsonString(pMessage, jsonString);
	//			lua_pushlstring(this->luaEnv, jsonString.c_str(), jsonString.size());
	//		}
	//		else
	//		{
	//			lua_pushlstring(this->luaEnv, message.c_str(), message.size());
	//		}
	//		if (lua_pcall(this->luaEnv, 1, 1, 0) != 0)
	//		{
	//			SayNoDebugError(lua_tostring(this->luaEnv, -1));
	//			return XCode::CallLuaFunctionFail;
	//		}
	//		argsCount++;
	//	}
	//
	//	if (lua_pcall(this->luaEnv, argsCount, 2, 0) != 0)
	//	{
	//		SayNoDebugError("call lua function error : " << lua_tostring(this->luaEnv, -1));
	//		return XCode::CallLuaFunctionFail;
	//	}
	//	XCode code = (XCode)lua_tointeger(this->luaEnv, -2);
	//	if (lua_isuserdata(this->luaEnv, -1))
	//	{
	//		std::string messageBuffer;
	//		Message * pRetMessage = (Message *)lua_touserdata(this->luaEnv, -1);
	//		if (pRetMessage != nullptr && pRetMessage->SerializePartialToString(&messageBuffer))
	//		{
	//			returnData->set_message_data(messageBuffer);
	//			returnData->set_protoc_name(pRetMessage->GetTypeName());
	//		}
	//	}
	//	else if (lua_istable(this->luaEnv, -1) && lua_getfunction(this->luaEnv, "JsonUtil", "ToString"))
	//	{
	//		lua_pushvalue(this->luaEnv, -2);
	//		if (lua_pcall(this->luaEnv, 1, 1, 0) != 0)
	//		{
	//			SayNoDebugError("call lua function error : " << lua_tostring(this->luaEnv, -1));
	//			return XCode::CallLuaFunctionFail;
	//		}
	//		size_t size = 0;
	//		const char * json = lua_tolstring(this->luaEnv, -1, &size);
	//		returnData->set_message_data(json, size);
	//	}
	//	return code;
	//}
}