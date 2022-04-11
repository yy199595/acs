//
// Created by yjz on 2022/4/4.
//

#ifndef _LUAREGISTERCOMPONENT_H_
#define _LUAREGISTERCOMPONENT_H_

#include"Component/Lua/LuaScriptComponent.h"

namespace Sentry
{
	class LuaRegisterComponent : public Component, public ILuaRegister
	{
	 public:
		LuaRegisterComponent() = default;
		~LuaRegisterComponent() = default;
	 public:
		bool LateAwake() final;
		void OnLuaRegister(lua_State *lua) final;
	 private:
		LuaScriptComponent * mLuaComponent;
	};
}

#endif //_LUAREGISTERCOMPONENT_H_
