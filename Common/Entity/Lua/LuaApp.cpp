//
// Created by yjz on 2022/5/15.
//

#include"LuaApp.h"
#include"App/App.h"
#include"Service/RpcService.h"
#include"Async/RpcTaskSource.h"
#include"Config/ServiceConfig.h"
#include"Component/ProtoComponent.h"
#include"Component/InnerNetMessageComponent.h"
using namespace Sentry;
namespace Lua
{
	int LuaApp::GetComponent(lua_State* lua)
	{
		const char* name = luaL_checkstring(lua, -1);
		Component* component = App::Inst()->GetComponentByName(name);
        if(component == nullptr)
        {
            return 0;
        }
		typedef UserDataParameter::UserDataStruct<Component*> ComponentType;
		return ComponentType::WriteObj(lua, component, name);
	}
}