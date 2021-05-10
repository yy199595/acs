#include"NetLuaAction.h"
#include<Other/ObjectFactory.h>

namespace SoEasy
{
	NetLuaAction::NetLuaAction(lua_State * lua, const std::string name, int action_ref, int invoke_ref)
	{
		this->luaEnv = lua;
		this->mActionName = name;
		this->mActionRef = action_ref;
		this->mInvokeRef = invoke_ref;
	}

	/*XCode NetLuaAction::Invoke1(shared_ptr<TcpClientSession> session, long long operatorId)
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
*/
	XCode NetLuaAction::Invoke(shared_ptr<TcpClientSession> session, const shared_ptr<NetWorkPacket> requestData)
	{
		lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->mInvokeRef);
		if (!lua_isfunction(this->luaEnv, -1))
		{
			return XCode::CallLuaFunctionFail;
		}
		lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->mActionRef);
		if (!lua_isfunction(this->luaEnv, -1))
		{
			return XCode::CallLuaFunctionFail;
		}
		LuaParameter::Write<shared_ptr<TcpClientSession>>(luaEnv, session);
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

		lua_State * luaCoroutine = lua_newthread(this->luaEnv);
		lua_pushvalue(luaEnv, 1);
		lua_xmove(luaEnv, luaCoroutine, 1);
		lua_replace(luaEnv, 1);

		const int size = lua_gettop(luaEnv);
		lua_xmove(luaEnv, luaCoroutine, size - 1);
		lua_resume(luaCoroutine, luaEnv, 1);
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