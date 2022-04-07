#pragma once

#include"LocalServerRpc.h"
#include"Script/LuaInclude.h"

namespace Sentry
{
	class LuaScriptComponent;

	class LuaRpcService : public LocalServerRpc, public IStart, public IHotfix
	{
	 public:
		LuaRpcService();
		~LuaRpcService() override;
	 public:
		bool Awake() final;
		void OnStart() final;
		void OnHotFix() final;
		bool LateAwake() final;
	 private:
		lua_State* mLuaEnv;
		class LuaScriptComponent* mLuaComponent;
	};
}// namespace Sentry