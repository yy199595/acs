#include "luaExtension.h"
#include <Core/Applocation.h>
#include <Manager/Manager.h>
#include <Module/Component.h>
#include <Object/GameObject.h>
#include <Util/StringHelper.h>

using namespace Sentry;
namespace LuaAPIExtension
{
    int GameObjectGetComponent(lua_State *lua)
    {
        SayNoAssertRetZero_F(lua_isuserdata(lua, 1));
        SayNoAssertRetZero_F(lua_isstring(lua, 2));
        GameObject *gameObject = PtrProxy<GameObject>::Read(lua, 1);
        if (gameObject)
        {
            const char *moduleName = lua_tostring(lua, 2);
            Component *component = gameObject->GetComponentByName(moduleName);
            if (component)
            {
                PtrProxy<Component>::Write(lua, component);
                lua_getglobal(lua, moduleName);
                lua_setmetatable(lua, -2);
                return 1;
            }
        }
        lua_pushnil(lua);
        return 1;
    }

    int ComponentGetComponent(lua_State *lua)
    {
        SayNoAssertRetZero_F(lua_isuserdata(lua, 1));
        SayNoAssertRetZero_F(lua_isstring(lua, 2));
        Component *component = PtrProxy<Component>::Read(lua, 1);
        if (component)
        {
            const char *moduleName = lua_tostring(lua, 2);
            SharedGameObject gameObject = component->GetGameObject();
            if (gameObject != nullptr)
            {
                Component *retComponent = gameObject->GetComponentByName(moduleName);
                PtrProxy<Component>::Write(lua, retComponent);
                lua_getglobal(lua, moduleName);
                lua_setmetatable(lua, -2);
                return 1;
            }
        }
        lua_pushnil(lua);
        return 1;
    }

    inline std::map<int, std::string> GetLuaStackData(lua_State *lua)
    {
        std::map<int, std::string> ret;

        int top = lua_gettop(lua);
        for (int index = 1; index <= top; index++)
        {
            std::string name = "";
            switch (lua_type(lua, -index))
            {
                case LUA_TNIL:
                    name = "nil";
                    ret.emplace(-index, "nil");
                    break;
                case LUA_TBOOLEAN:
                    ret.emplace(-index, "nil");
                    break;
                case LUA_TLIGHTUSERDATA:
                    ret.emplace(-index, "lightuserdata");
                    break;
                case LUA_TNUMBER:
                    ret.emplace(-index, "number");
                    break;
                case LUA_TSTRING:
                    ret.emplace(-index, "string");
                    break;
                case LUA_TTABLE:
                    ret.emplace(-index, "table");
                    break;
                case LUA_TFUNCTION:
                    ret.emplace(-index, "function");
                    break;
                case LUA_TUSERDATA:
                    ret.emplace(-index, "userdata");
                    break;
                case LUA_TTHREAD:
                    ret.emplace(-index, "thread");
                    break;
            }
        }
        return ret;
    }

    int GetComponent(lua_State *lua)
    {
        SayNoAssertRetZero_F(lua_isuserdata(lua, -2));
        SayNoAssertRetZero_F(lua_isstring(lua, -1));
        const char *name = lua_tostring(lua, -1);
        GameObject *gameObject = PtrProxy<GameObject>::Read(lua, -2);
        if (gameObject)
        {
            Component *component = gameObject->GetComponentByName(name);
            if (component)
            {
                PtrProxy<Component>::Write(lua, component);
                lua_getglobal(lua, name);
                lua_setmetatable(lua, -2);
                return 1;
            }
        }
        lua_pushnil(lua);
        return 1;
        return 0;
    }

    int AddComponent(lua_State *lua)
    {
        SayNoAssertRetZero_F(lua_isuserdata(lua, -2));
        SayNoAssertRetZero_F(lua_isstring(lua, -1));
        const char *name = lua_tostring(lua, -1);
        GameObject *gameObject = PtrProxy<GameObject>::Read(lua, -2);
        if (gameObject)
        {
            Component *component = gameObject->AddComponentByName(name);
            if (component != nullptr)
            {
                PtrProxy<Component>::Write(lua, component);
                lua_getglobal(lua, name);
                lua_setmetatable(lua, -2);
                return 1;
            }
        }
        lua_pushnil(lua);
        return 1;
    }

    int TypeCast(lua_State *luaEnv)
    {
        if (lua_isuserdata(luaEnv, 1) && lua_isstring(luaEnv, 2))
        {
            const char *name = lua_tostring(luaEnv, 2);
            lua_pop(luaEnv, 1);
            if (luaL_newmetatable(luaEnv, name) != 0)
            {
                lua_setmetatable(luaEnv, -2);
            }
            return 1;
        }
        return 0;
    }

    extern void GetLuaString(lua_State *luaEnv, std::string &outstring)
    {
        lua_Debug ar;
        if (lua_getstack(luaEnv, 1, &ar) == 1)
        {
            lua_getinfo(luaEnv, "nSlu", &ar);
            int n = lua_gettop(luaEnv);
            lua_getglobal(luaEnv, "tostring");
            std::stringstream stringBuffer;

            const char *file = ar.short_src;


            stringBuffer << StringHelper::GetFileName(ar.short_src) << ":" << ar.currentline << "  ";
            for (int i = 1; i <= n; i++)
            {
                size_t size;
                lua_pushvalue(luaEnv, -1);
                lua_pushvalue(luaEnv, i);
                lua_call(luaEnv, 1, 1);
                const char *str = lua_tolstring(luaEnv, -1, &size);
                outstring.append(str, size);
                outstring.append(" ");
                lua_pop(luaEnv, 1);
                stringBuffer << str << " ";
            }
            outstring = stringBuffer.str();
        }
    }

    int DebugLog(lua_State *luaEnv)
    {
        std::string printString;
        GetLuaString(luaEnv, printString);
        SayNoCommonLog(printString);
        return 0;
    }

    int DebugInfo(lua_State *luaEnv)
    {
        std::string printString;
        GetLuaString(luaEnv, printString);
        SayNoCommonInfo(printString);
        return 0;
    }

    int DebugError(lua_State *luaEnv)
    {
        std::string printString;
        GetLuaString(luaEnv, printString);
        SayNoCommonError(printString);
        return 0;
    }

    int DebugWarning(lua_State *luaEnv)
    {
        std::string printString;
        GetLuaString(luaEnv, printString);
        SayNoCommonWarning(printString);
        return 0;
    }
}// namespace LuaAPIExtension