#include "LuaFunction.h"
#include <Define/CommonLogDef.h>
LuaFunction::LuaFunction(lua_State *luaEnv, int ref)
{
    this->luaEnv = luaEnv;
    this->ref = ref;
}

std::shared_ptr<LuaFunction> LuaFunction::Create(lua_State *luaEnv, const std::string & name)
{
    lua_getglobal(luaEnv, name.c_str());
    if (lua_isfunction(luaEnv, -1))
    {
        int ref = luaL_ref(luaEnv, LUA_REGISTRYINDEX);
        return std::make_shared<LuaFunction>(luaEnv, ref);
    }
    return nullptr;
}

std::shared_ptr<LuaFunction> LuaFunction::Create(lua_State *luaEnv, const std::string & tabName, const std::string & name)
{
    if(lua_getfunction(luaEnv, tabName.c_str(), name.c_str()))
    {
        int ref = luaL_ref(luaEnv, LUA_REGISTRYINDEX);
        return std::make_shared<LuaFunction>(luaEnv, ref);
    }
    return nullptr;
}
