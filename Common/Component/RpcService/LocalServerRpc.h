//
// Created by mac on 2022/4/6.
//

#ifndef SERVER_LOCALSERVERRPC_H
#define SERVER_LOCALSERVERRPC_H
#include"RpcServiceNode.h"
#include"Method/MethodRegister.h"
#include"Component/Lua/LuaScriptComponent.h"

namespace Sentry
{
	class LocalServerRpc : public RpcServiceNode
	{
	public:
		LocalServerRpc() = default;

	protected:
		virtual bool OnInitService(ServiceMethodRegister & methodRegister) = 0;
	public:
		bool LoadService() final;
		bool IsStartService() { return this->mMethodRegister != nullptr; }
		std::shared_ptr<com::Rpc_Response> Invoke(const std::string& method, std::shared_ptr<com::Rpc_Request> request);
	protected:
		std::shared_ptr<ServiceMethodRegister> mMethodRegister;
	};
}
#endif //SERVER_LOCALSERVERRPC_H
