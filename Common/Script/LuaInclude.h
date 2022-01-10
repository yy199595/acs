#pragma once

#include <assert.h>
#include <iostream>
#include <memory>
#include <string.h>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
};

#include "ClassNameProxy.h"
#include "ParameterType.h"

inline std::string FromatFileAndLine(const char *file, const int line)
{
    const size_t lenght = strlen(file);
    const char *fileName = nullptr;

    for (size_t index = lenght - 1; index >= 0; index--)
    {
#ifdef _WIN32
        if (file[index] == '\\')
#else
        if (file[index] == '/')
#endif
        {
            fileName = file + index + 1;
            break;
        }
    }
    fileName = fileName == nullptr ? file : fileName;
    char buffer[100] = {};
#ifdef _MSC_VER
    size_t size = sprintf_s(buffer, "%s:%d", fileName, line);
#else
    size_t size = sprintf(buffer, "%s:%d", fileName, line);
#endif// _MSG
    return std::string(buffer, size);
}


#define LuaDebugLog(msg)                                                                                            \
    {                                                                                                               \
        std::cout << "[Lua Error] : " << FromatFileAndLine(__FILE__, __LINE__) << "  [" << msg << "]" << std::endl; \
    }
#define LuaAssertLog(code, msg)                   \
    {                                             \
        if (code != 0) LuaDebugLog(#code << msg); \
    }
#define LuaAssertReturn(code, msg) \
    {                              \
        if (code != 0)             \
        {                          \
            LuaDebugLog(msg);      \
            return;                \
        }                          \
    }
#define LuaAssertReturnValue(code, msg, val)                     \
    {                                                            \
        if (code != 0) { LuaDebugLog(#code << msg) return val; } \
    }
#define LuaAssertReturnZero(code, msg)                    \
    {                                                     \
        if (code) { LuaDebugLog(#code << msg) return 0; } \
    }                                                     \






inline bool lua_getfunction(lua_State *lua, int idx, const char *func)
{
    if (lua_istable(lua, idx))
    {
        lua_getfield(lua, idx, func);
        if (lua_isfunction(lua, -1))
        {
            lua_remove(lua, -2);
            return true;
        }
        lua_remove(lua, -2);
    }
    return false;
}
inline bool lua_getfunction(lua_State *lua, const char *tab, const char *func)
{
    lua_getglobal(lua, tab);
    return lua_getfunction(lua, -1, func);
}


inline int lua_reffunction(lua_State *lua, const char *tab, const char *func)
{
    lua_getglobal(lua, tab);
    if (lua_istable(lua, -1))
    {
        lua_getfield(lua, -1, func);
        if (lua_isfunction(lua, -1))
        {
            lua_remove(lua, -2);
            return luaL_ref(lua, LUA_REGISTRYINDEX);
        }
        lua_remove(lua, -2);
    }
    return 0;
}



#define lua_ref(lua) luaL_ref(lua, LUA_REGISTRYINDEX)
#define lua_unref(lua, ref) luaL_unref(lua, LUA_REGISTRYINDEX, ref)
#define lua_getref(lua, ref) lua_rawgeti(lua, LUA_REGISTRYINDEX, ref)
#define lua_presume(cor, lua, args)  \
{ \
	int code = lua_resume(cor, lua, args); \
	if(code != 0 && code != LUA_YIELD) \
	{	\
		LOG_ERROR(lua_tostring(cor, -1);)\
	} \
}\

inline void lua_pushtablefunction(lua_State *lua, const char *name, lua_CFunction func)
{
    lua_pushcfunction(lua, func);
    lua_setfield(lua, -2, name);
}

inline void lua_pushglobalfunction(lua_State *lua, const char *name, lua_CFunction func)
{
    lua_pushcfunction(lua, func);
    lua_setglobal(lua, name);
}

//template<typename T>
//struct TypeProxy {
//    typedef T Type;
//};
//
//template<typename T>
//struct TypeProxy<T *> {
//    typedef T Type;
//};
//
//template<typename T>
//struct TypeProxy<T &> {
//    typedef T Type;
//};
//
//template<typename T>
//struct TypeProxy<std::shared_ptr<T>> {
//    typedef T Type;
//};

namespace LuaParameter
{
    /*template<typename T>
    inline typename std::enable_if<ConstParameter::IsConstParameter<T>::value, T>::type Read(lua_State * lua, int index);
    template<typename T>
    inline typename std::enable_if<ConstParameter::IsConstParameter<T>::value, void>::type Write(lua_State * lua, T data);*/

    template<typename T>
    inline typename std::enable_if<CommonParameter::IsCommonParameter<T>::value, T>::type
    Read(lua_State *lua, int index);

    template<typename T>
    inline typename std::enable_if<CommonParameter::IsCommonParameter<T>::value, void>::type
    Write(lua_State *lua, T data);

    template<typename T>
    inline typename std::enable_if<ContainerParameter::IsContainerParameter<T>::value, T>::type
    Read(lua_State *lua, int index);

    template<typename T>
    inline typename std::enable_if<ContainerParameter::IsContainerParameter<T>::value, void>::type
    Write(lua_State *lua, T data);

    template<typename T>
    inline typename std::enable_if<FunctionParameter::IsFunctionParameter<T>::value, T>::type
    Read(lua_State *lua, int index);

    template<typename T>
    inline typename std::enable_if<FunctionParameter::IsFunctionParameter<T>::value, void>::type
    Write(lua_State *lua, T data);

    template<typename T>
    inline typename std::enable_if<UserDataParameter::IsUserDataParameter<T>::value, T>::type
    Read(lua_State *lua, int index);

    template<typename T>
    inline typename std::enable_if<UserDataParameter::IsUserDataParameter<T>::value, void>::type
    Write(lua_State *lua, T data);

    template<typename T>
    inline typename std::enable_if<ProtocParameter::IsProtocParameter<T>::value, T>::type
    Read(lua_State *lua, int index);

    template<typename T>
    inline typename std::enable_if<ProtocParameter::IsProtocParameter<T>::value, void>::type
    Write(lua_State *lua, T data);

    template<typename T, typename... Args>
    inline void Encode(lua_State *lua, const T &t, Args... args);

    template<typename... Args>
    inline void WriteArgs(lua_State *lua, Args... args);
}// namespace LuaParameter


template<typename T>
struct PtrProxy {
public:
    PtrProxy(T *t) : mNativePtr(t), mIsDestory(false) {}

    PtrProxy(T *t, bool isDestory) : mNativePtr(t), mIsDestory(isDestory) {}

    ~PtrProxy() {}

public:
    static T *Read(lua_State *lua, int index)
    {
        if (lua_isuserdata(lua, index))
        {
            PtrProxy<T> *p = (PtrProxy<T> *) (lua_touserdata(lua, index));
            return p->mNativePtr;
        }
        return nullptr;
    }

    static void Write(lua_State *lua, T *data)
    {
        if (data == nullptr) return;
        new (lua_newuserdata(lua, sizeof(PtrProxy<T>))) PtrProxy<T>(data, false);
    }


    static void Destory(lua_State *lua, int index)
    {
        if (lua_isuserdata(lua, index))
        {
            PtrProxy<T> *p = (PtrProxy<T> *) (lua_touserdata(lua, index));
            if (p != nullptr)
            {
                if (p->mIsDestory)
                {
                    delete p->mNativePtr;
                    return;
                }
                p->~PtrProxy();
            }
        }
    }

private:
    T *mNativePtr;
    bool mIsDestory;
};


template<typename T>
struct SharedPtrProxy {
public:
    SharedPtrProxy(std::shared_ptr<T> t) : mNativePtr(t) {}

    ~SharedPtrProxy() {}

public:
    static std::shared_ptr<T> Read(lua_State *lua, int index)
    {
        if (lua_isuserdata(lua, index))
        {
            SharedPtrProxy<T> *p = (SharedPtrProxy<T> *) (lua_touserdata(lua, index));
            return p->mNativePtr;
        }
        return nullptr;
    }

    static void Write(lua_State *lua, std::shared_ptr<T> data)
    {
        if (data == nullptr) return;
        new (lua_newuserdata(lua, sizeof(SharedPtrProxy<T>))) SharedPtrProxy<T>(data, false);
    }

    static void Write(lua_State *lua, T *data)
    {
        if (data == nullptr) return;
        new (lua_newuserdata(lua, sizeof(PtrProxy<T>))) PtrProxy<T>(data, false);
    }

    static void Destory(lua_State *lua, int index)
    {
        if (lua_isuserdata(lua, index))
        {
            SharedPtrProxy<T> *p = (SharedPtrProxy<T> *) (lua_touserdata(lua, index));
            if (p != nullptr)
            {
                p->~SharedPtrProxy();
            }
        }
    }

private:
    std::shared_ptr<T> mNativePtr;
};