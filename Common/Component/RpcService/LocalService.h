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
	class LocalRpcService : public Service
	{
	public:
		LocalRpcService() = default;
	protected:
		virtual void OnCloseService() { }
		virtual bool OnStartService(ServiceMethodRegister & methodRegister) = 0;
	public:
		bool StartNewService() final;
		bool CloseService() final;
		bool IsStartService() { return this->mMethodRegister != nullptr; }
		XCode Invoke(const std::string &func, std::shared_ptr<com::rpc::request>, std::shared_ptr<com::rpc::response> response) final;
	private:
		std::shared_ptr<ServiceMethodRegister> mMethodRegister;
	};
}
#endif //SERVER_LOCALSERVICECOMPONENT_H
