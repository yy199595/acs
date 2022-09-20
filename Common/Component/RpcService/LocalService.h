//
// Created by mac on 2022/4/6.
//

#ifndef SERVER_LOCALSERVICECOMPONENT_H
#define SERVER_LOCALSERVICECOMPONENT_H
#include"Service.h"
#include"Method/MethodRegister.h"
#include"Component/Lua/LuaScriptComponent.h"

namespace Sentry
{
	class LocalService : public Service
	{
	public:
		LocalService() = default;
	protected:
		virtual void OnCloseService() { }
		virtual bool OnStartService() = 0;
	public:
		bool StartNewService() final;
		bool CloseService() final;
		bool IsStartService() { return this->mMethodRegister != nullptr; }
		XCode Invoke(const std::string &func, std::shared_ptr<com::rpc::request>, std::shared_ptr<com::rpc::response> response) final;
    protected:
        ServiceMethodRegister & GetMethodRegistry() { return *this->mMethodRegister; }
	private:
		std::shared_ptr<ServiceMethodRegister> mMethodRegister;
	};
    extern std::string GET_FUNC_NAME(std::string fullName);
#define BIND_COMMON_RPC_METHOD(func) this->GetMethodRegistry().Bind(GET_FUNC_NAME(#func), &func);
#define BIND_ADDRESS_RPC_METHOD(func) this->GetMethodRegistry().BindAddress(GET_FUNC_NAME(#func), &func);
}
#endif //SERVER_LOCALSERVICECOMPONENT_H
