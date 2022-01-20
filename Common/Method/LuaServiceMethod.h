#pragma once
#include "ServiceMethod.h"
struct lua_State;
namespace Sentry
{

	class ProtoConfig;
	class LuaServiceMethod : public ServiceMethod
	{
	public:
		LuaServiceMethod(const ProtoConfig * config, lua_State * lua, int idx);
	public:
		bool IsLuaMethod() final { return true; }
		XCode Invoke(const com::Rpc_Request & request, com::Rpc_Response & response) final;
	private:
		tuple<XCode, std::shared_ptr<Message>> Call(long long id, const std::string & json);
        tuple<XCode, std::shared_ptr<Message>> CallAsync(long long id, const std::string & json);
    private:
		int mIdx;
		lua_State * mLuaEnv;
        const ProtoConfig * mProtoConfig;
		class LuaScriptComponent * mScriptComponent;
		class RpcClientComponent * mRpcClientComponent;
	};
}