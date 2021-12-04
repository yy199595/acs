#include "LuaFunction.h"
#include <Define/CommonLogDef.h>
LuaFunction::LuaFunction(lua_State *luaEnv, int ref)
{
    this->luaEnv = luaEnv;
    this->ref = ref;
}

LuaFunction *LuaFunction::Create(lua_State *luaEnv, const std::string & name)
{
    lua_getglobal(luaEnv, name.c_str());
    if (lua_isfunction(luaEnv, -1))
    {
        int ref = luaL_ref(luaEnv, LUA_REGISTRYINDEX);
        return new LuaFunction(luaEnv, ref);
    }
    return nullptr;
}

LuaFunction *LuaFunction::Create(lua_State *luaEnv, const std::string & tabName, const std::string & name)
{
    lua_getglobal(luaEnv, tabName.c_str());
    if (lua_istable(luaEnv, -1))
    {
        lua_getfield(luaEnv, -1, name.c_str());
        if (lua_isfunction(luaEnv, -1))
        {
            int ref = luaL_ref(luaEnv, LUA_REGISTRYINDEX);
            return new LuaFunction(luaEnv, ref);
        }
    }
    return nullptr;
}

void LuaFunction::Action()
{
    lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->ref);
    if (lua_isfunction(this->luaEnv, -1))
    {
        if(lua_pcall(this->luaEnv, 0, 0, 0) != 0)
        {
            LOG_ERROR(lua_tostring(luaEnv, -1));
        }
    }
}
