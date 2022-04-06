#pragma once


#include"Script/LuaInclude.h"
#include"Component/Service/RpcServiceBase.h"

class LuaTable;
namespace Sentry
{
	class LuaScriptComponent;

	class LuaRpcService : public RpcServiceBase, public IStart, public IHotfix
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