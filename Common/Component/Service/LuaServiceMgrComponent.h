#pragma once
#include <Component/Component.h>
struct lua_State;
namespace GameKeeper
{
	class LuaServiceMethod;
    class LuaServiceMgrComponent : public Component, public IHotfix
	{
	public:
        LuaServiceMgrComponent() = default;
        ~LuaServiceMgrComponent() final = default;
	protected:
		bool Awake() final;
        void OnHotFix() final;
        bool LateAwake() final;
    private:
        class LuaScriptComponent * mLuaComponent;
        class RpcConfigComponent * mRpcConfigComponent;
	};
}