//
// Created by yjz on 2022/5/15.
//

#include"LuaApp.h"
#include"App/App.h"
#include"Component/RpcService/Service.h"
using namespace Sentry;
namespace Lua
{
	int LuaApp::GetService(lua_State* lua)
	{
		luaL_checkstring(lua, -1);
		const char * meta = "Service";
		const char* name = luaL_checkstring(lua, -1);
		Service* localServiceComponent = App::Get()->GetComponent<Service>(name);
		return UserDataParameter::UserDataStruct<Service*>::WriteObj(lua, localServiceComponent, meta);
	}
	int LuaApp::GetComponent(lua_State* lua)
	{
		luaL_checkstring(lua, -1);
		const char* name = luaL_checkstring(lua, -1);
		Component* component = App::Get()->GetComponentByName(name);
		return UserDataParameter::UserDataStruct<Component*>::WriteObj(lua, component, name);
	}
}