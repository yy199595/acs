#pragma once

#include <Define/CommonDef.h>
#include <Script/LuaFunction.h>

class LuaTable
{
public:
    LuaTable(lua_State *luaEnv, int ref);

    LuaTable(lua_State *luaEnv, int index, bool isPos);

    ~LuaTable();

public:
    static LuaTable *Create(lua_State *luaEnv, const std::string name);

public:
    template<typename T>
    T GetMemberVariable(const char *name);

public:
    template<typename Ret, typename... Args>
    bool Function(const char *func, Ret &retValue, Args &&...args);

    template<typename... Args>
    bool Action(const char *func, Args &&...args);

    bool Serialization(Message &message);

    bool Serialization(std::string &outString);


private:
    int ref;
    lua_State *luaEnv;
    std::string tableName;
};

template<typename T>
T LuaTable::GetMemberVariable(const char *name)
{
    lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->ref);
    if (lua_istable(this->luaEnv, -1))
    {
        lua_getfield(this->luaEnv, -1, name);
        return LuaParameter::Read<T>(this->luaEnv, -1);
    }
    return T();
}

template<typename Ret, typename... Args>
inline bool LuaTable::Function(const char *func, Ret &retValue, Args &&...args)
{
    lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->ref);
    if (!lua_istable(this->luaEnv, -1))
    {
        return false;
    }
    lua_getfield(this->luaEnv, -1, func);
    if (!lua_isfunction(this->luaEnv, -1))
    {
        return false;
    }
    LuaParameter::WriteArgs<Args...>(this->luaEnv, std::forward<Args>(args)...);
    if (lua_pcall(this->luaEnv, sizeof...(Args), 1, 0) != 0)
    {
        GKDebugError(this->tableName << "." << func << ":" << lua_tostring(this->luaEnv, -1));
        return false;
    }
    retValue = LuaParameter::Read<Ret>(this->luaEnv, -1);
    return true;
}

template<typename... Args>
inline bool LuaTable::Action(const char *func, Args &&...args)
{
    lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->ref);
    if (!lua_istable(this->luaEnv, -1))
    {
        return false;
    }
    lua_getfield(this->luaEnv, -1, func);
    if (!lua_isfunction(this->luaEnv, -1))
    {
        return false;
    }
    LuaParameter::WriteArgs<Args...>(this->luaEnv, std::forward<Args>(args)...);
    if (lua_pcall(this->luaEnv, sizeof...(Args), 0, 0) != 0)
    {
        GKDebugError(this->tableName << "." << func << ":" << lua_tostring(this->luaEnv, -1));
        return false;
    }
    return true;
}
