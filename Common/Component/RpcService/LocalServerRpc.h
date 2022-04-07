//
// Created by mac on 2022/4/6.
//

#ifndef SERVER_LOCALSERVERRPC_H
#define SERVER_LOCALSERVERRPC_H
#include"RpcServiceComponent.h"
#include"RpcMethodBinder.h"
#include"Component/Lua/LuaScriptComponent.h"
namespace Sentry
{
	class LocalServerRpc : public RpcServiceComponent, public RpcMethodBinder, public ILuaRegister
	{
	public:
		LocalServerRpc() = default;
	public:
		void OnLuaRegister(lua_State *lua) final;
		std::shared_ptr<com::Rpc_Response> Invoke(const std::string& method, std::shared_ptr<com::Rpc_Request> request);
	};
#define BIND_RPC_FUNCTION(func) LOG_CHECK_RET_FALSE(this->Bind(this->GetName(), GetFunctionName(#func), &func))
}
#endif //SERVER_LOCALSERVERRPC_H
