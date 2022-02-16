#pragma once

#include"Object/App.h"
#include<Define/CommonLogDef.h>
#include"Script/LuaParameter.h"
using namespace Sentry;
class LuaFunction
{
public:
    LuaFunction(lua_State *luaEvn, int ref);

    ~LuaFunction() { luaL_unref(luaEnv, LUA_REGISTRYINDEX, this->ref); }

public:
    int GetRef() { return this->ref;}
    static std::shared_ptr<LuaFunction> Create(lua_State *luaEnv, const std::string & name);

    static std::shared_ptr<LuaFunction> Create(lua_State *luaEnv, const std::string & tabName, const std::string & name);

public:

    template<typename Ret, typename... Args>
    Ret Func(Args... args);

    template<typename... Args>
    void Action(Args... args);

    inline int GetFunctionRef() const { return this->ref; }

private:
    int ref;
    lua_State *luaEnv;
};

template<typename... Args>
inline void LuaFunction::Action(Args... args)
{
    lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->ref);
    LuaParameter::WriteArgs<Args...>(this->luaEnv, std::forward<Args>(args)...);
    if (lua_pcall(this->luaEnv, sizeof...(Args), 0, 0) != 0)
    {
        throw std::logic_error(lua_tostring(luaEnv, -1));
    }
}


template<typename Ret, typename... Args>
inline Ret LuaFunction::Func(Args... args)
{
    lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->ref);
    LuaParameter::WriteArgs<Args...>(this->luaEnv, std::forward<Args>(args)...);
    if (lua_pcall(this->luaEnv, sizeof...(Args), 1, 0) != 0)
    {
        throw std::logic_error(lua_tostring(luaEnv, -1));
    }
    return LuaParameter::Read<Ret>(this->luaEnv, -1);
}

