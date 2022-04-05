//
// Created by yjz on 2022/4/4.
//


#include"LuaRegisterComponent.h"
#include"Script/LuaLogger.h"
#include"Script/ClassProxyHelper.h"
#include"Script/Timer/Timer.h"
#include"Script/Coroutine/LuaCoroutine.h"
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

		Lua::ClassProxyHelper::BeginNewTable(lua, "Timer");
		Lua::ClassProxyHelper::PushStaticExtensionFunction(lua, "Timer", "AddTimer", Lua::Timer::AddTimer);
		Lua::ClassProxyHelper::PushStaticExtensionFunction(lua, "Timer", "CancelTimer", Lua::Timer::CancelTimer);

		Lua::ClassProxyHelper::PushStaticExtensionFunction(lua, "coroutine", "sleep", Lua::Coroutine::Sleep);
		Lua::ClassProxyHelper::PushStaticExtensionFunction(lua, "coroutine", "start", Lua::Coroutine::Start);

	}
}