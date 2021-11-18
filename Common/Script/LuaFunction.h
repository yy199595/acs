#pragma once

#include <Define/CommonDef.h>
#include <Script/LuaInclude.h>

class LuaFunction
{
public:
    LuaFunction(lua_State *luaEvn, int ref);

    ~LuaFunction() { luaL_unref(luaEnv, LUA_REGISTRYINDEX, this->ref); }

public:
    static LuaFunction *Create(lua_State *luaEnv, const std::string & name);

    static LuaFunction *Create(lua_State *luaEnv, const std::string & tabName, const std::string & name);

public:
    template<typename Ret>
    Ret Func();

    template<typename Ret, typename... Args>
    Ret Func(Args... args);

    void Action();

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
    if (lua_isfunction(this->luaEnv, -1))
    {
        LuaParameter::WriteArgs<Args...>(this->luaEnv, std::forward<Args>(args)...);
        int status = lua_pcall(this->luaEnv, sizeof...(Args), 0, 0);
        GKAssertRet(status == 0, lua_tostring(luaEnv, -1));
    }
}


template<typename Ret, typename... Args>
inline Ret LuaFunction::Func(Args... args)
{
    lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->ref);
    if (lua_isfunction(this->luaEnv, -1))
    {
        LuaParameter::WriteArgs<Args...>(this->luaEnv, std::forward<Args>(args)...);
        int status = lua_pcall(this->luaEnv, sizeof...(Args), 1, 0);
        GKAssertRetVal(status == 0, lua_tostring(luaEnv, -1), Ret());
        return LuaParameter::Read<Ret>(this->luaEnv, -1);
    }
    return Ret();
}

template<typename Ret>
inline Ret LuaFunction::Func()
{
    lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->ref);
    if (lua_isfunction(this->luaEnv, -1))
    {
        int status = lua_pcall(this->luaEnv, 0, 1, 0);
        GKAssertRetVal(status == 0, lua_tostring(luaEnv, -1), Ret());
        return LuaParameter::Read<Ret>(this->luaEnv, -1);
    }
    return Ret();
}
