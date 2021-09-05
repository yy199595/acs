#include "LuaServiceMethod.h"
#include <Script/LuaInclude.h>
#include <Core/App.h>
#include <Scene/SceneNetProxyComponent.h>
#include <Scene/SceneProtocolComponent.h>
namespace Sentry
{

	LuaServiceMethod::LuaServiceMethod(const std::string & name, lua_State * lua, int idx)
		:ServiceMethod(name), mLuaEnv(lua), mIdx(idx)
	{
		this->mProtocolComponent = Scene::GetComponent<SceneProtocolComponent>();
		SayNoAssertBreakFatal_F(this->mProtocolComponent);
	}

	XCode LuaServiceMethod::Invoke(PacketMapper *messageData)
	{
		lua_State *coroutine = lua_newthread(this->mLuaEnv);

		lua_getfunction(mLuaEnv, "Service", "Invoke");
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
			Message * message = this->mProtocolComponent->CreateMessage(protocolConfig->RequestMsgName);
			if (message == nullptr || !message->ParseFromString(messageData->GetMsgBody()))
			{
				SayNoDebugFatal("Init request message failure");
				return XCode::Failure;
			}		
			this->mProtocolComponent->GetJsonByMessage(message, mMessageJson);
			lua_pushlstring(coroutine, mMessageJson.c_str(), mMessageJson.size());
		}
		int top = lua_gettop(coroutine);
		lua_resume(coroutine, this->mLuaEnv, top - 1);
		return XCode::Successful;
	}
	int LuaServiceMethod::Response(lua_State * lua)
	{
		XCode code = (XCode)lua_tointeger(lua, 2);
		PacketMapper * responseData = (PacketMapper*)lua_touserdata(lua, 1);
		if (responseData->SetCode(code))
		{
			responseData->ClearMessage();
			const std::string & responseName = responseData->GetProConfig()->ResponseMsgName;
			if (lua_isstring(lua, -3) && !responseName.empty())
			{
				SceneProtocolComponent * protocolComponent = Scene::GetComponent<SceneProtocolComponent>();
				Message * message = protocolComponent->CreateMessage(responseName);
				SayNoAssertBreakFatal_F(responseData->SetMessage(message));
			}
			SceneNetProxyComponent * component = Scene::GetComponent<SceneNetProxyComponent>();
			component->SendNetMessage(responseData);
		}
		return 0;
	}
}
