#pragma once
#include "Component/Component.h"
struct lua_State;
namespace GameKeeper
{
	class LuaServiceMethod;
    class LuaServiceMgrComponent : public Component, public IHotfix
	{
	public:
        LuaServiceMgrComponent() = default;
        ~LuaServiceMgrComponent() final = default;
        int GetPriority() final { return 10000;}
	protected:
		bool Awake() final;
        void OnHotFix() final;
        bool LateAwake() final;
	};
}