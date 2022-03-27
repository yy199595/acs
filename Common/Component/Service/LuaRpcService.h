#pragma once


#include"Script/LuaInclude.h"
#include"Component/Service/RpcService.h"

class LuaTable;
namespace Sentry
{
	class LuaScriptComponent;

	class LuaRpcService : public RpcService, public IStart, public IHotfix
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
		class RpcConfigComponent* mConfigComponent;
	};
}// namespace Sentry