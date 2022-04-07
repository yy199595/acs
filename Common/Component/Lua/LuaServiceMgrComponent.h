#pragma once
#include"Component/Component.h"
struct lua_State;
namespace Sentry
{
	class LuaRpcService;
	class LuaServiceMethod;
	class LuaServiceMgrComponent : public Component, public IHotfix
	{
	 public:
		LuaServiceMgrComponent() = default;
		~LuaServiceMgrComponent() final = default;

	 public:
		bool AddLuaMethod(class LocalServerRpc* rpcService);
	 protected:
		bool Awake() final;
		void OnHotFix() final;
		bool LateAwake() final;

	 private:
		class LuaScriptComponent* mLuaComponent;
		class RpcConfigComponent* mConfigComponent;
	};
}