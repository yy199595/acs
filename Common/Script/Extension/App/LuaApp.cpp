//
// Created by yjz on 2022/5/15.
//

#include"LuaApp.h"
#include"App/App.h"
#include"Component/RpcService/LocalServiceComponent.h"
using namespace Sentry;
namespace Lua
{
	int LuaApp::GetService(lua_State* lua)
	{
		luaL_checkstring(lua, -1);
		const char* name = lua_tostring(lua, -1);
		LocalServiceComponent* localServiceComponent = App::Get()->GetComponent<LocalServiceComponent>(name);
		return UserDataParameter::UserDataStruct<LocalServiceComponent*>::WriteObj(lua, localServiceComponent, name);
	}
	int LuaApp::GetComponent(lua_State* lua)
	{
		luaL_checkstring(lua, -1);
		const char* name = lua_tostring(lua, -1);
		Component* component = App::Get()->GetComponentByName(name);
		return UserDataParameter::UserDataStruct<Component*>::WriteObj(lua, component, name);
	}
}