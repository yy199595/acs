#pragma once
#include "ServiceMethod.h"
struct lua_State;
namespace GameKeeper
{

	class ProtocolConfig;
	class LuaServiceMethod : public ServiceMethod
	{
	public:
		LuaServiceMethod(const std::string & name, lua_State * lua, int idx);
	public:
		bool IsLuaMethod() final { return true; }
		XCode AsyncInvoke(const com::Rpc_Request & request);
		XCode Invoke(const com::Rpc_Request & request, com::Rpc_Response & response) final;
	private:
		static int Response(lua_State * lua);
	private:
		int mIdx;
		lua_State * mLuaEnv;
		std::string mMessageJson;
		class LuaScriptComponent * mScriptComponent;
		class RpcClientComponent * mRpcClientComponent;
	};
}