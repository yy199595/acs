#pragma once

#include"LocalServiceComponent.h"
#include"Script/LuaInclude.h"

namespace Sentry
{
	class LuaScriptComponent;

	class LocalLuaServiceComponent : public LocalServiceComponent, public IStart
	{
	 public:
		LocalLuaServiceComponent();
		~LocalLuaServiceComponent() override;
	 public:
		bool OnStart() final;
		bool LateAwake() final;
		bool OnStartService(ServiceMethodRegister & methodRegister) final;
	 private:
		lua_State* mLuaEnv;
		class LuaScriptComponent* mLuaComponent;
	};
}// namespace Sentry