//
// Created by 64658 on 2023/3/19.
//

#include"LuaGate.h"
#include"Entity/Actor/App.h"
#include"Lua/Engine/UserDataParameter.h"
#include"Gate/Component/GateComponent.h"
using namespace Tendo;
using namespace Lua::UserDataParameter;
namespace Lua
{
    int Gate::Send(lua_State *lua)
    {
        GateComponent *component = App::Inst()->GetComponent<GateComponent>();
        if (component == nullptr)
        {
            luaL_error(lua, "not find gate component");
            return 0;
        }
        long long userId = luaL_checkinteger(lua, 1);
        const char *func = luaL_checkstring(lua, 2);
        std::shared_ptr<Message> message = UserDataStruct<std::shared_ptr<Message>>::Read(lua,3);
        if(message == nullptr)
        {
            lua_pushboolean(lua, component->Send(userId, func));
            return 1;
        }
        lua_pushboolean(lua, component->Send(userId, func, *message));
        return 1;
    }

    int Gate::BroadCast(lua_State *lua)
    {
        GateComponent *component = App::Inst()->GetComponent<GateComponent>();
        if (component == nullptr)
        {
            luaL_error(lua, "not find gate component");
            return 0;
        }
        const char *func = luaL_checkstring(lua, 1);
        std::shared_ptr<Message> message = UserDataStruct<std::shared_ptr<Message>>::Read(lua,2);
        if(message == nullptr)
        {
            lua_pushinteger(lua, component->BroadCast(func));
            return 1;
        }
        lua_pushinteger(lua, component->BroadCast(func, *message));
        return 1;
    }
}