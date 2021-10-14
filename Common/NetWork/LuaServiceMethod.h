#pragma once
#include "ServiceMethod.h"
struct lua_State;
namespace Sentry
{
	
	class ProtocolConfig;
	class LuaServiceMethod : public ServiceMethod
	{
	public:
		LuaServiceMethod(const std::string & name, lua_State * lua, int idx);
	public:
		bool IsLuaMethod() final { return true; }
		 XCode Invoke(const com::DataPacket_Request & request, std::string & response) final;
         XCode AsyncInvoke(const com::DataPacket_Request & request);
	private:
		static int Response(lua_State * lua);
	private:
		int mIdx;
		lua_State * mLuaEnv;
		std::string mMessageJson;
		class LuaScriptComponent * mScriptComponent;
		class ProtocolComponent * mProtocolComponent;
	};
}