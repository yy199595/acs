#pragma once

#include "ClassNameProxy.h"
#include "LuaDebugStack.h"
#include "LuaInclude.h"

class LuaManager
{
public:
    LuaManager()
        : luaEnv(nullptr)
    {
        errorFunctonRef = 0;
        luaEnv = luaL_newstate();
        luaL_openlibs(luaEnv);
    }

    void AddRequirePath(const std::string path);

    bool LoadLuaFile(const std::string filePath);

    template<typename T>
    bool PushGlobalVariable(const std::string name, T *data);

    inline lua_State *GetLuaEnv() { return this->luaEnv; }

private:
    lua_State *luaEnv;
    int errorFunctonRef;
};

template<typename T>
bool LuaManager::PushGlobalVariable(const std::string name, T *data)
{
    const char *mateName = ClassNameProxy::GetLuaClassName<T>();
    if (mateName != nullptr)
    {
        PtrProxy<T>::Write(luaEnv, data);
        lua_getglobal(luaEnv, mateName);
        if (lua_istable(luaEnv, -1))
        {
            lua_setmetatable(luaEnv, -2);
            lua_setglobal(luaEnv, name.c_str());
            return true;
        }
    }
    return false;
}
