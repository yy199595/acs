#include "SystemExtension.h"
#include <Core/App.h>
#include <Scene/ActionComponent.h>
#include <Service/ServiceMgrComponent.h>
#include <NetWork/NetWorkRetAction.h>
#include <Service/LuaServiceComponent.h>
#include <Timer/LuaActionTimer.h>
#include <Timer/LuaSleepTimer.h>
#include <Timer/TimerComponent.h>
#include <Service/ServiceNodeComponent.h>
#include <Scene/LuaScriptComponent.h>
#include <Scene/ProtocolComponent.h>

#include <Service/ServiceNode.h>
#include <Pool/MessagePool.h>
using namespace Sentry;

int SystemExtension::Call(lua_State *lua)
{
	luaL_checktype(lua, 1, LUA_TNUMBER);
	luaL_checktype(lua, 2, LUA_TSTRING);
	luaL_checktype(lua, 3, LUA_TSTRING);
	const int nodeId = lua_tointeger(lua, 1);
	const char * service = lua_tostring(lua, 2);
	const char * method = lua_tostring(lua, 3);

	if (lua_pushthread(lua) == 1)
	{
		lua_pushinteger(lua, XCode::NoCoroutineContext);
		SayNoDebugError("call " << service << "." << method << " not coroutine context");
		return 1;
	}
	lua_State * coroutine = lua_tothread(lua, -1);

	ServiceNodeComponent * nodeComponent = App::Get().GetComponent<ServiceNodeComponent>();

	ServiceNode * serviceNode = nodeComponent->GetServiceNode(nodeId);
	if (serviceNode == nullptr || !serviceNode->HasService(service))
	{
		lua_pushinteger(lua, XCode::CallServiceNotFound);
		return 1;
	}
	int index = 4;
	PacketMapper * packetMapper = nullptr;
	if (lua_isinteger(lua, index))
	{
		const long long userId = lua_tointeger(lua, index);
//		packetMapper = PacketMapper::Create(serviceNode->GetAddress(), S2C_REQUEST, service, method);
//		if (packetMapper == nullptr)
//		{
//			lua_pushinteger(lua, XCode::CallArgsError);
//			return 1;
//		}
//		index++;
//		packetMapper->SetUserId(userId);
	}
	else
	{
		//packetMapper = PacketMapper::Create(serviceNode->GetAddress(), S2S_REQUEST, service, method);
	}
//
//	const ProtocolConfig * config = packetMapper->GetProConfig();
//	if (lua_istable(lua, index))
//	{
//		LuaScriptComponent * scriptComponent = App::Get().GetComponent<LuaScriptComponent>();
//		int ref = scriptComponent->GetLuaRef("Json", "ToString");
//		lua_rawgeti(lua, LUA_REGISTRYINDEX, ref);
//		lua_pushvalue(lua, index);
//		if (lua_pcall(lua, 1, 1, 0) != 0)
//		{
//			SayNoDebugError("call " << service << "." << method << " " << lua_tostring(lua, -1));
//			lua_pushinteger(lua, (int)XCode::Failure);
//			return 1;
//		}
//
//		size_t size = 0;
//		const char * json = lua_tolstring(lua, -1, &size);
//		Message * message = MessagePool::NewByJson(config->RequestMessage, json, size);
//		if (message != nullptr)
//		{
//			if (!packetMapper->SetMessage(message))
//			{
//				lua_pushinteger(lua, (int)XCode::ProtocbufCastJsonFail);
//				return 1;
//			}
//		}
//		else
//		{
//			packetMapper->SetMessage(json, size);
//		}
//	}
//
//	auto actionComponent = App::Get().GetComponent<ActionComponent>();
//	auto cb = std::make_shared<LocalWaitRetActionProxy>(lua, coroutine);
//
//	if (!packetMapper->SetRpcId(actionComponent->AddCallback(cb)))
//	{
//		lua_pushinteger(lua, (int)XCode::Failure);
//		return 1;
//	}
//
//	serviceNode->AddMessageToQueue(packetMapper, false);
	
	return lua_yield(lua, 1);
}

int SystemExtension::GetApp(lua_State *luaEnv)
{
    App * app = &App::Get();
    LuaParameter::Write<App *>(luaEnv, app);
    return 1;
}

int SystemExtension::GetManager(lua_State *luaEnv)
{
    if (lua_isstring(luaEnv, -1))
    {
        App &app = App::Get();
        const std::string name = lua_tostring(luaEnv, -1);
        //TODO
        PtrProxy<Manager>::Write(luaEnv, nullptr);
        if (lua_getglobal(luaEnv, name.c_str()) && lua_istable(luaEnv, -1))
        {
            lua_setmetatable(luaEnv, -2);
            return 1;
        }
    }
    return 0;
}

extern bool SystemExtension::RequireLua(lua_State *luaEnv, const char *name)
{
    lua_getglobal(luaEnv, "require");
    if (lua_isfunction(luaEnv, -1))
    {
        lua_pushstring(luaEnv, name);
        if (lua_pcall(luaEnv, 1, 1, 0) != 0)
        {
            SayNoDebugError(lua_tostring(luaEnv, -1));
            return false;
        }
        return lua_istable(luaEnv, -1);
    }
    return false;
}

int SystemExtension::Sleep(lua_State *luaEnv)
{
    long long ms = lua_tointeger(luaEnv, 1);
    lua_pushthread(luaEnv);
    TimerComponent *pTimerManager = App::Get().GetComponent<TimerComponent>();
    if (pTimerManager != nullptr)
    {
        shared_ptr<TimerBase> pTimer = LuaSleepTimer::Create(luaEnv, -1, ms);
        pTimerManager->AddTimer(pTimer);
    }
    return lua_yield(luaEnv, 1);
}

int SystemExtension::AddTimer(lua_State *lua)
{
    if (!lua_isfunction(lua, 1))
    {
        return 0;
    }
    if (!lua_isinteger(lua, 2))
    {
        return 0;
    }
    int count = 1;
    int interval = lua_tointeger(lua, 2);
    if (lua_isinteger(lua, 3))
    {
        count = lua_tointeger(lua, 3);
    }
    TimerComponent *pTimerManager = App::Get().GetComponent<TimerComponent>();
    if (pTimerManager != nullptr)
    {
        lua_pushvalue(lua, 1);
        if (!lua_isfunction(lua, -1))
        {
            return 0;
        }
        int ref = luaL_ref(lua, LUA_REGISTRYINDEX);
        shared_ptr<LuaActionTimer> sharedTimer = pTimerManager->CreateTimer<LuaActionTimer>(lua, ref, interval, count);
        if (sharedTimer != nullptr)
        {
            lua_pushinteger(lua, sharedTimer->GetTimerId());
            return 1;
        }
    }
    return 0;
}

int SystemExtension::RemoveTimer(lua_State *lua)
{
    if (lua_isinteger(lua, 1))
    {
        long long id = lua_tointeger(lua, 1);
        TimerComponent *pTimerManager = App::Get().GetComponent<TimerComponent>();
        if (pTimerManager != nullptr)
        {
            bool code = pTimerManager->RemoveTimer(id);
            lua_pushboolean(lua, code);
            return 1;
        }
    }
    lua_pushboolean(lua, false);
    return 1;
}
