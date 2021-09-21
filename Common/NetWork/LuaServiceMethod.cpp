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
		this->mScriptComponent = Scene::GetComponent<LuaScriptComponent>();
		this->mProtocolComponent = Scene::GetComponent<ProtocolComponent>();
		SayNoAssertBreakFatal_F(this->mProtocolComponent);
	}

	XCode LuaServiceMethod::Invoke(PacketMapper *messageData)
	{
		lua_State *coroutine = lua_newthread(this->mLuaEnv);
		int ref = this->mScriptComponent->GetLuaRef("Service", "Invoke");
		//lua_getfunction(mLuaEnv, "Service", "Invoke");
		lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, ref);
		luaL_checktype(this->mLuaEnv, -1, LUA_TFUNCTION);

		lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->mIdx);	
		luaL_checktype(this->mLuaEnv, -1, LUA_TFUNCTION);

		mMessageJson.clear();
		lua_xmove(this->mLuaEnv, coroutine, 2);
		lua_pushcfunction(coroutine, LuaServiceMethod::Response);
		lua_pushlightuserdata(coroutine, messageData);
		lua_pushinteger(coroutine, messageData->GetUserId());
		const ProtocolConfig * protocolConfig = messageData->GetProConfig();
		if (!protocolConfig->RequestMsgName.empty())
		{		
			const std::string & data = messageData->GetMsgBody();
			const std::string & name = protocolConfig->RequestMsgName;
			
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
		return XCode::Successful;
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
		NetProxyComponent * sessionComponent = Scene::GetComponent<NetProxyComponent>();

		responseData->ClearMessage();
		if (lua_isstring(lua, 3))
		{
			size_t size = 0;
			const char * json = lua_tolstring(lua, 3, &size);
			const std::string & responseName = responseData->GetProConfig()->ResponseMsgName;
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