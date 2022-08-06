#pragma once


#include"Script/LuaInclude.h"
#include"Service.h"

namespace Sentry
{
	class LuaScriptComponent;
	class ServiceMethodRegister;
	class LuaService : public Service, public IStart
	{
	 public:
		LuaService();
		~LuaService() override;
	 protected:
		bool OnStart() final;
		bool LateAwake() final;
	 public:
		bool StartNewService() final;
		bool CloseService() final;
		bool IsStartService() final { return this->mMethodRegister != nullptr; }
		XCode Invoke(const std::string& name, std::shared_ptr<com::rpc::request> request,
			std::shared_ptr<com::rpc::response> response) final;
	 private:
		lua_State* mLuaEnv;
		class LuaScriptComponent* mLuaComponent;
		std::shared_ptr<ServiceMethodRegister> mMethodRegister;
	};
}// namespace Sentry