//
// Created by mac on 2022/4/6.
//

#ifndef SERVER_LOCALSERVICECOMPONENT_H
#define SERVER_LOCALSERVICECOMPONENT_H
#include"ServiceCallComponent.h"
#include"Method/MethodRegister.h"
#include"Component/Lua/LuaScriptComponent.h"

namespace Sentry
{
	class LocalRpcServiceBase : public ServiceCallComponent, public IServiceBase
	{
	public:
		LocalRpcServiceBase() = default;
	protected:
		virtual void OnCloseService() { }
		virtual bool OnStartService(ServiceMethodRegister & methodRegister) = 0;
	public:
		bool StartService() final;
		bool CloseService() final;
		bool IsStartService() { return this->mMethodRegister != nullptr; }
		bool IsStartComplete() final { return this->GetAddressSize() > 0; }
		XCode Invoke(const std::string &func, std::shared_ptr<com::Rpc::Request>, std::shared_ptr<com::Rpc::Response> response);
	private:
		std::shared_ptr<ServiceMethodRegister> mMethodRegister;
	};
}
#endif //SERVER_LOCALSERVICECOMPONENT_H
