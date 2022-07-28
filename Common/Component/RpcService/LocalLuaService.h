#pragma once


#include"Script/LuaInclude.h"
#include"ServiceComponent.h"

namespace Sentry
{
	class LuaScriptComponent;
	class ServiceMethodRegister;
	class LocalLuaService : public ServiceComponent, public IStart
	{
	 public:
		LocalLuaService();
		~LocalLuaService() override;
	 protected:
		bool OnStart() final;
		bool LateAwake() final;
	 public:
		bool StartNewService() final;
		bool CloseService() final;
		bool IsStartService() final { return this->mMethodRegister != nullptr; }
		XCode Invoke(const std::string& name, std::shared_ptr<com::Rpc::Request> request,
			std::shared_ptr<com::Rpc::Response> response) final;
	 private:
		lua_State* mLuaEnv;
		class LuaScriptComponent* mLuaComponent;
		std::shared_ptr<ServiceMethodRegister> mMethodRegister;
	};
}// namespace Sentry