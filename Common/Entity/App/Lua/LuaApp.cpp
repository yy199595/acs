//
// Created by yjz on 2022/5/15.
//

#include"LuaApp.h"
#include"App/App.h"
#include"Service/RpcService.h"
using namespace Sentry;
namespace Lua
{
	int LuaApp::GetService(lua_State* lua)
	{
		luaL_checkstring(lua, -1);
		const char * meta = "RpcService";
		const char* name = luaL_checkstring(lua, -1);
		RpcService* localServiceComponent = App::Inst()->GetComponent<RpcService>(name);
		return UserDataParameter::UserDataStruct<RpcService*>::WriteObj(lua, localServiceComponent, meta);
	}
	int LuaApp::GetComponent(lua_State* lua)
	{
		luaL_checkstring(lua, -1);
		const char* name = luaL_checkstring(lua, -1);
		Component* component = App::Inst()->GetComponentByName(name);
		return UserDataParameter::UserDataStruct<Component*>::WriteObj(lua, component, name);
	}
}