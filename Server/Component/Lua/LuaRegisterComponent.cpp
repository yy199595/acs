//
// Created by yjz on 2022/4/4.
//


#include"LuaRegisterComponent.h"
#include"Script/LuaLogger.h"
#include"Script/ClassProxyHelper.h"
#include"Async/LuaTaskSource.h"
namespace Sentry
{
	bool Sentry::LuaRegisterComponent::Awake()
	{
		this->mLuaComponent = nullptr;
		return true;
	}
	bool LuaRegisterComponent::LateAwake()
	{
		this->mLuaComponent = this->GetComponent<LuaScriptComponent>();
		return true;
	}
	void LuaRegisterComponent::OnLuaRegister(lua_State* lua)
	{
		Lua::ClassProxyHelper::PushStaticExtensionFunction(lua, "Log", "Debug", Lua::Log::DebugLog);
		Lua::ClassProxyHelper::PushStaticExtensionFunction(lua, "Log", "Warning", Lua::Log::DebugWarning);
		Lua::ClassProxyHelper::PushStaticExtensionFunction(lua, "Log", "Error", Lua::Log::DebugError);
		Lua::ClassProxyHelper::PushStaticExtensionFunction(lua, "Log", "Info", Lua::Log::DebugInfo);
	}
}