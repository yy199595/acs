//
// Created by yjz on 2022/5/15.
//

#include"LuaApp.h"
#include"App/App.h"
#include"Service/RpcService.h"
using namespace Sentry;
namespace Lua
{
	int LuaApp::GetComponent(lua_State* lua)
	{
		luaL_checkstring(lua, -1);
		const char* name = luaL_checkstring(lua, -1);
		Component* component = App::Inst()->GetComponentByName(name);
        if(component == nullptr)
        {
            lua_pushnil(lua);
            return 1;
        }
        const std::string fullName = fmt::format("App{0}", name);
		return UserDataParameter::UserDataStruct<Component*>::WriteObj(lua, component, fullName.c_str());
	}
}