#include "LuaServiceMgrComponent.h"
#include <Core/App.h>
#include <NetWork/LuaServiceMethod.h>
#include <Service/LuaServiceComponent.h>
#include <Scene/LuaScriptComponent.h>
#include <Scene/ProtocolComponent.h>
#include <Util/DirectoryHelper.h>
namespace Sentry
{
	bool LuaServiceMgrComponent::Awake()
	{
		LuaScriptComponent * scriptComponent = Scene::GetComponent<LuaScriptComponent>();
		ProtocolComponent * protoComponent = Scene::GetComponent<ProtocolComponent>();

		string servicePath;
		SayNoAssertRetFalse_F(protoComponent);
		SayNoAssertRetFalse_F(scriptComponent);
		lua_State * lua = scriptComponent->GetLuaEnv();

		std::vector<std::string> services;
		protoComponent->GetServices(services);

		GameObject & serviceObject = App::Get().Scene;
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
		
			ServiceComponent * localService = serviceObject.GetComponent<ServiceComponent>(service);
			if (localService == nullptr)
			{
				LuaServiceComponent * luaSerivce = new LuaServiceComponent();
				if (serviceObject.AddComponent(service, localService))
				{
					SayNoDebugError("add service " << service << " failure");
					return false;
				}
				if (!luaSerivce->Init(service) || !luaSerivce->InitService(service, lua))
				{
					SayNoDebugFatal("Init lua service [" << service << "] failure");
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
				SayNoDebugInfo("add new lua service method : " << service << "." << method);
			}
		}
		return true;
	}
}