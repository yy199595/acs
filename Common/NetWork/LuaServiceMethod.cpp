#include "LuaServiceMethod.h"
#include <Script/LuaInclude.h>
#include <Core/App.h>
#include <Scene/LuaScriptComponent.h>
#include <Scene/NetProxyComponent.h>
#include <Scene/ProtocolComponent.h>
#include <Pool/MessagePool.h>
namespace Sentry
{

	LuaServiceMethod::LuaServiceMethod(const std::string & name, lua_State * lua, int idx)
		:ServiceMethod(name), mLuaEnv(lua), mIdx(idx)
	{
		this->mScriptComponent = App::Get().GetComponent<LuaScriptComponent>();
		this->mProtocolComponent = App::Get().GetComponent<ProtocolComponent>();
		SayNoAssertBreakFatal_F(this->mProtocolComponent);
	}

	XCode LuaServiceMethod::Invoke(PacketMapper *messageData)
	{
		lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, mIdx);
		luaL_checktype(this->mLuaEnv, -1, LUA_TFUNCTION);
		lua_pushinteger(this->mLuaEnv, messageData->GetUserId());
		const ProtocolConfig * config = messageData->GetProConfig();
		if (!messageData->GetMsgBody().empty())
		{
			int ref = this->mScriptComponent->GetLuaRef("Json", "ToObject");
			lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, ref);
			luaL_checktype(this->mLuaEnv, -1, LUA_TFUNCTION);

			const std::string & data = messageData->GetMsgBody();
			if (!config->Request.empty())
			{
				Message * message = MessagePool::NewByData(config->Request, data);
				if (message == nullptr)
				{
					return XCode::ParseMessageError;
				}
				string json = "";
				if (!util::MessageToJsonString(*message, &json).ok())
				{
					return XCode::ProtocbufCastJsonFail;
				}
				lua_pushlstring(this->mLuaEnv, json.c_str(), json.size());
			}
			else
			{
				lua_pushlstring(this->mLuaEnv, data.c_str(), data.size());
			}
			if (lua_pcall(this->mLuaEnv, 1, 1, 0) != 0)
			{
				SayNoDebugError(lua_tostring(this->mLuaEnv, -1));
				return XCode::CallLuaFunctionFail;
			}
			lua_remove(this->mLuaEnv, -2);
		}

		if (lua_pcall(this->mLuaEnv, 2, 2, 0) != 0)
		{
			SayNoDebugError(lua_tostring(this->mLuaEnv, -1));
			return XCode::CallLuaFunctionFail;
		}

		XCode code = (XCode)lua_tointeger(this->mLuaEnv, -2);
		if (lua_istable(this->mLuaEnv, -1) && code == XCode::Successful)
		{
			int ref = this->mScriptComponent->GetLuaRef("Json", "ToString");
			lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, ref);
			luaL_checktype(this->mLuaEnv, -1, LUA_TFUNCTION);
			lua_pushvalue(this->mLuaEnv, -3);
			if (lua_pcall(this->mLuaEnv, 1, 1, 0) != 0)
			{
				SayNoDebugError(lua_tostring(this->mLuaEnv, -1));
				return XCode::CallLuaFunctionFail;
			}
			size_t size = 0;
			const char * json = lua_tolstring(this->mLuaEnv, -1, &size);
			Message * message = MessagePool::NewByJson(config->Response, json, size);

			message != nullptr ? messageData->SetMessage(*message) :
				messageData->SetMessage(json, size);
		}

		return code;
	}

	XCode LuaServiceMethod::AsyncInvoke(PacketMapper *messageData)
	{
		lua_State *coroutine = lua_newthread(this->mLuaEnv);
		int ref = this->mScriptComponent->GetLuaRef("Service", "Invoke");
		//lua_getfunction(mLuaEnv, "Service", "Invoke");
		lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, ref);
		//luaL_checktype(this->mLuaEnv, -1, LUA_TFUNCTION);

		if (!lua_isfunction(this->mLuaEnv, -1))
		{
			return XCode::CallLuaFunctionFail;
		}

		lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->mIdx);	
		//luaL_checktype(this->mLuaEnv, -1, LUA_TFUNCTION);
		if (!lua_isfunction(this->mLuaEnv, -1))
		{
			return XCode::CallLuaFunctionFail;
		}

		mMessageJson.clear();
		lua_xmove(this->mLuaEnv, coroutine, 2);
		lua_pushcfunction(coroutine, LuaServiceMethod::Response);
		lua_pushlightuserdata(coroutine, messageData);
		lua_pushinteger(coroutine, messageData->GetUserId());
		const ProtocolConfig * protocolConfig = messageData->GetProConfig();
		if (!protocolConfig->Request.empty())
		{		
			const std::string & data = messageData->GetMsgBody();
			const std::string & name = protocolConfig->Request;
			
			Message * message = MessagePool::NewByData(name, data);
			if (message == nullptr)
			{
				SayNoDebugFatal("Init request message failure");
				return XCode::ParseMessageError;
			}
			if (!util::MessageToJsonString(*message, &mMessageJson).ok())
			{
				return XCode::ProtocbufCastJsonFail;
			}
			lua_pushlstring(coroutine, mMessageJson.c_str(), mMessageJson.size());
		}
		else if (messageData->GetMsgBody().size() > 0)
		{
			const size_t size = messageData->GetMsgBody().size();
			const char * json = messageData->GetMsgBody().c_str();		
			lua_pushlstring(coroutine, json, size);
		}
		int top = lua_gettop(coroutine);
		lua_presume(coroutine, this->mLuaEnv, top - 1);
		return XCode::LuaCoroutineWait;
	}
	int LuaServiceMethod::Response(lua_State * lua)
	{
		XCode code = (XCode)lua_tointeger(lua, 2);
		PacketMapper * responseData = (PacketMapper*)lua_touserdata(lua, 1);
		if (!responseData->SetCode(code))
		{
			responseData->Destory();
			return 0;
		}
		NetProxyComponent * sessionComponent = App::Get().GetComponent<NetProxyComponent>();

		responseData->ClearMessage();
		if (lua_isstring(lua, 3))
		{
			size_t size = 0;
			const char * json = lua_tolstring(lua, 3, &size);
			const std::string & responseName = responseData->GetProConfig()->Response;
			Message * message = MessagePool::NewByJson(responseName, json, size);
			if (message != nullptr)
			{
				SayNoAssertBreakFatal_F(message);
				SayNoAssertBreakFatal_F(responseData->SetMessage(message));
			}
			else
			{
				responseData->SetMessage(json, size);
			}
		}	
		sessionComponent->SendNetMessage(responseData);

		return 0;
	}
}
