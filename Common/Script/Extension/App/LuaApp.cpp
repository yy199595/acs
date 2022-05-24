//
// Created by yjz on 2022/5/15.
//

#include"LuaApp.h"
#include"App/App.h"
#include"Component/RpcService/ServiceCallComponent.h"
using namespace Sentry;
namespace Lua
{
	int LuaApp::GetService(lua_State* lua)
	{
		luaL_checkstring(lua, -1);
		const char * meta = "ServiceCallComponent";
		const char* name = lua_tostring(lua, -1);
		ServiceCallComponent* localServiceComponent = App::Get()->GetComponent<ServiceCallComponent>(name);
		return UserDataParameter::UserDataStruct<ServiceCallComponent*>::WriteObj(lua, localServiceComponent, meta);
	}
	int LuaApp::GetComponent(lua_State* lua)
	{
		luaL_checkstring(lua, -1);
		const char* name = lua_tostring(lua, -1);
		Component* component = App::Get()->GetComponentByName(name);
		return UserDataParameter::UserDataStruct<Component*>::WriteObj(lua, component, name);
	}
}