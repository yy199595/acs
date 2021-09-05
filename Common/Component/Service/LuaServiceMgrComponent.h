#pragma once
#include <Component/Component.h>
struct lua_State;
namespace Sentry
{
	class LuaServiceMethod;
	class LuaServiceMgrComponent : public Component
	{
	public:
		
	protected:
		bool Awake() final;
	};
}