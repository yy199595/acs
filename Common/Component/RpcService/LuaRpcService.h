#pragma once

#include"LocalServerRpc.h"
#include"Script/LuaInclude.h"

namespace Sentry
{
	class LuaScriptComponent;

	class LuaRpcService : public LocalServerRpc, public IStart
	{
	 public:
		LuaRpcService();
		~LuaRpcService() override;
	 public:
		bool OnStart() final;
		bool LateAwake() final;
		bool OnInitService(ServiceMethodRegister & methodRegister) final;
	 private:
		lua_State* mLuaEnv;
		class LuaScriptComponent* mLuaComponent;
	};
}// namespace Sentry