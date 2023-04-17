//
// Created by yjz on 2022/5/15.
//

#include"LuaApp.h"
#include"Entity/Unit/App.h"
#include"Rpc/Service/RpcService.h"
#include"Rpc/Async/RpcTaskSource.h"
#include"Proto/Component/ProtoComponent.h"
#include"Rpc/Component/DispatchComponent.h"
using namespace Tendo;
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