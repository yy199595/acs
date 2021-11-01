#include "LuaServiceMgrComponent.h"
#include <Core/App.h>
#include <Method/LuaServiceMethod.h>
#include <Service/LuaServiceComponent.h>
#include <Scene/LuaScriptComponent.h>
#include <Scene/ProtocolComponent.h>
#include <Util/DirectoryHelper.h>
namespace GameKeeper
{
	bool LuaServiceMgrComponent::Awake()
	{
		LuaScriptComponent * scriptComponent = this->GetComponent<LuaScriptComponent>();
		ProtocolComponent * protoComponent = this->GetComponent<ProtocolComponent>();

		string servicePath;
		GKAssertRetFalse_F(protoComponent);
		GKAssertRetFalse_F(scriptComponent);
		lua_State * lua = scriptComponent->GetLuaEnv();

		std::vector<std::string> services;
		protoComponent->GetServices(services);

		for (std::string & service : services)
		{
			lua_getglobal(lua, service.c_str());
			if (!lua_istable(lua, -1))
			{
				continue;
			}
			std::vector<std::string> methods;
			if (!protoComponent->GetMethods(service, methods))
			{
				continue;
			}
		
			ServiceComponent * localService = App::Get().GetComponent<ServiceComponent>(service);
			if (localService == nullptr)
			{
				LuaServiceComponent * luaSerivce = new LuaServiceComponent();
				if (App::Get().AddComponent(service, localService))
				{
					GKDebugError("add service " << service << " failure");
					return false;
				}
				if (!luaSerivce->Init(service) || !luaSerivce->InitService(service, lua))
				{
					GKDebugFatal("Init lua service [" << service << "] failure");
					return false;
				}
				localService = luaSerivce;
			}

			int ref = luaL_ref(lua, LUA_REGISTRYINDEX);
			for (std::string & method : methods)
			{
				lua_rawgeti(lua, LUA_REGISTRYINDEX, ref);
				lua_getfield(lua, -1, method.c_str());
				if (!lua_isfunction(lua, -1))
				{
					continue;
				}
				int idx = luaL_ref(lua, LUA_REGISTRYINDEX);
				localService->AddMethod(new LuaServiceMethod(method, lua, idx));
				GKDebugInfo("add new lua service method : " << service << "." << method);
			}
		}
		return true;
	}
}