#include "SystemExtension.h"
#include "Object/App.h"
#include "Async/RpcTask/RpcTaskSource.h"
#include <Timer/LuaSleepTimer.h>
#include "Component/Timer/TimerComponent.h"
#include "Component/Scene/ServiceMgrComponent.h"
#include"Component/Lua/LuaScriptComponent.h"
#include"Service/ServiceProxy.h"
using namespace Sentry;

int SystemExtension::Call(lua_State *lua)
{

    return 0;
}

int SystemExtension::Allot(lua_State *luaEnv)
{
    App & app = App::Get();
    luaL_checkstring(luaEnv, -1);
    ServiceMgrComponent * serviceMgrComponent = app.GetComponent<ServiceMgrComponent>();
    auto serviceProxy = serviceMgrComponent->GetServiceProxy(lua_tostring(luaEnv, -1));


    return 0;
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
            LOG_ERROR(lua_tostring(luaEnv, -1));
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
    auto *timerComponent = App::Get().GetComponent<TimerComponent>();
    if (timerComponent != nullptr)
    {
        auto timer = LuaSleepTimer::Create(luaEnv, -1, ms);
        timerComponent->AddTimer(timer);
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
    App & app = App::Get();
    auto timerComponent = app.GetComponent<TimerComponent>();
    if (timerComponent != nullptr)
    {
        lua_pushvalue(lua, 1);
        if (!lua_isfunction(lua, -1))
        {
            return 0;
        }
        int ref = luaL_ref(lua, LUA_REGISTRYINDEX);
        //TODO
        //shared_ptr<LuaActionTimer> sharedTimer = pTimerManager->CreateTimer<LuaActionTimer>(lua, ref, interval, count);
        //if (sharedTimer != nullptr)
        {
            //lua_pushinteger(lua, sharedTimer->GetTimerId());
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
            bool code = pTimerManager->CancelTimer(id);
            lua_pushboolean(lua, code);
            return 1;
        }
    }
    lua_pushboolean(lua, false);
    return 1;
}
