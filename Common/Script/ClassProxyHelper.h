
#pragma once

#include<cstring>
#include<typeinfo>
#include"LuaParameter.h"
#include"CtorFunction.h"
#include"ClassMateProxy.h"
#include"MemberFuncProxy.h"
#include"StaticFuncProxy.h"

class ClassProxyHelper
{
public:
    template<typename T>
    static void BeginRegister(lua_State *lua, const char *name);

    template<typename T, typename ... Args>
    static void PushCtor(lua_State *lua);

    template<typename T, typename Base>
    static void PushBaseClass(lua_State *lua);

    template<typename T, typename Ret, typename ... Args>
    static void PushMemberFunction(lua_State *lua, const char *name, Ret(T::*func)(Args ...));

    template<typename T, typename Ret, typename ... Args>
    static void PushMemberFunction(lua_State *lua, const char *name, Ret(T::*func)(Args ...) const);

    template<typename T, typename BC, typename Ret, typename ... Args>
    static void PushBaseMemberFunction(lua_State *lua, const char *name, Ret(BC::*func)(Args ...));

    template<typename T>
    static void PushExtensionFunction(lua_State *lua, const char *name, lua_CFunction func);

    static void PushGlobalExtensionFunction(lua_State *lua, const char *name, lua_CFunction func);

public:

    static void BeginNewTalbe(lua_State *lua, const char *table);

    static void PushStaticExtensionFunction(lua_State *lua, const char *table, const char *name, lua_CFunction func);

    template<typename Ret, typename ... Args>
    static void PushStaticFunction(lua_State *lua, const char *table, const char *name, Ret(*func)(Args ...));
};

template<typename T, typename ... Args>
void ClassProxyHelper::PushCtor(lua_State *luaEnv)
{
    const char *name = ClassNameProxy::GetLuaClassName<T>();
    lua_getglobal(luaEnv, name);
    if (lua_istable(luaEnv, -1))
    {
        lua_pushstring(luaEnv, "New");
        lua_pushcclosure(luaEnv, CtorFunction::PushCtor<T, Args ...>, 0);
        lua_settable(luaEnv, -3);
    }
    lua_pop(luaEnv, 1);
}

template<typename T, typename Base>
inline void ClassProxyHelper::PushBaseClass(lua_State *luaEnv)
{
    if (!ClassNameProxy::HasRegisterClass<Base>())
    {
        printf("[Lua Error] : The parent class is not registered %s\n", typeid(Base).name());
        return;
    }
    if (!std::is_base_of<Base, T>::value)
    {
        printf("[Lua Error] : T Is not a subclass %s\n", typeid(Base).name());
        return;
    }
    ClassNameProxy::OnPushParent<T, Base>();
}

template<typename T>
void ClassProxyHelper::BeginRegister(lua_State *luaEnv, const char *name)
{
    lua_getglobal(luaEnv, name);
    if (lua_isnil(luaEnv, -1))
    {
        lua_newtable(luaEnv);
        {
            lua_pushstring(luaEnv, "__index");
            lua_pushcclosure(luaEnv, ClassMateProxy::OnMateTableGet<T>, 0);
            lua_rawset(luaEnv, -3);

            lua_pushstring(luaEnv, "__gc");
            lua_pushcclosure(luaEnv, ClassMateProxy::OnDestory<T>, 0);
            lua_rawset(luaEnv, -3);

            lua_pushstring(luaEnv, "__name");
            lua_pushstring(luaEnv, name);
            lua_rawset(luaEnv, -3);

        }
        lua_setglobal(luaEnv, name);
        ClassNameProxy::OnClassRegister<T>(name);
    }
}

template<typename T>
inline void ClassProxyHelper::PushExtensionFunction(lua_State *luaEnv, const char *funcName, lua_CFunction func)
{
    const char *name = ClassNameProxy::GetLuaClassName<T>();
    lua_getglobal(luaEnv, name);
    if (lua_istable(luaEnv, -1))
    {
        lua_pushstring(luaEnv, funcName);
        lua_pushcfunction(luaEnv, func);
        lua_rawset(luaEnv, -3);
    }
    lua_pop(luaEnv, 1);
}

template<typename T, typename Ret, typename ... Args>
void ClassProxyHelper::PushMemberFunction(lua_State *luaEnv, const char *name, Ret(T::*func)(Args ...))
{
    const char *className = ClassNameProxy::GetLuaClassName<T>();
    typedef Ret(T::*MemberFunctionType)(Args ...);
    lua_getglobal(luaEnv, className);
    if (lua_istable(luaEnv, -1))
    {
        lua_pushstring(luaEnv, name);
        new(lua_newuserdata(luaEnv, sizeof(MemberFunctionType))) MemberFunctionType(func);
        lua_pushcclosure(luaEnv, MemberFunction::MemberFuncProxy<T, Ret, Args...>::Invoke, 1);
        lua_settable(luaEnv, -3);
    }
    lua_pop(luaEnv, 1);
}

template<typename T, typename Ret, typename ...Args>
inline void ClassProxyHelper::PushMemberFunction(lua_State *luaEnv, const char *name, Ret(T::* func)(Args...) const)
{
    const char *className = ClassNameProxy::GetLuaClassName<T>();
    typedef Ret(T::*MemberFunctionType)(Args ...) const;
    lua_getglobal(luaEnv, className);
    if (lua_istable(luaEnv, -1))
    {
        lua_pushstring(luaEnv, name);
        new(lua_newuserdata(luaEnv, sizeof(MemberFunctionType))) MemberFunctionType(func);
        lua_pushcclosure(luaEnv, MemberFunction::MemberFuncProxy<T, Ret, Args...>::Invoke, 1);
        lua_settable(luaEnv, -3);
    }
    lua_pop(luaEnv, 1);
}

template<typename T, typename BC, typename Ret, typename ...Args>
inline void ClassProxyHelper::PushBaseMemberFunction(lua_State *luaEnv, const char *name, Ret(BC::* func)(Args...))
{
    const char *className = ClassNameProxy::GetLuaClassName<T>();
    typedef Ret(BC::*MemberFunctionType)(Args ...);
    lua_getglobal(luaEnv, className);
    if (lua_istable(luaEnv, -1))
    {
        lua_pushstring(luaEnv, name);
        new(lua_newuserdata(luaEnv, sizeof(MemberFunctionType))) MemberFunctionType(func);
        lua_pushcclosure(luaEnv, MemberFunction::MemberFuncProxy<BC, Ret, Args...>::Invoke, 1);
        lua_settable(luaEnv, -3);
    }
    lua_pop(luaEnv, 1);
}

template<typename Ret, typename ... Args>
void ClassProxyHelper::PushStaticFunction(lua_State *luaEnv, const char *table, const char *name, Ret(*func)(Args ...))
{
    typedef Ret(*StaticFunctionType)(Args ...);
    lua_getglobal(luaEnv, table);
    if (lua_istable(luaEnv, -1))
    {
        lua_pushstring(luaEnv, name);
        new(lua_newuserdata(luaEnv, sizeof(StaticFunctionType))) StaticFunctionType(func);
        lua_pushcclosure(luaEnv, StaticFunction::StaticFuncProxy<Ret, Args ...>::Invoke, 1);
        lua_settable(luaEnv, -3);
    }
    lua_pop(luaEnv, 1);
}
