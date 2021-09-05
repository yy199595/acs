#include "LuaServiceMgrComponent.h"
#include <Core/App.h>
#include <NetWork/LuaServiceMethod.h>
#include <Service/LuaServiceProxy.h>
#include <Scene/SceneScriptComponent.h>
#include <Scene/SceneProtocolComponent.h>
#include <Util/DirectoryHelper.h>
namespace Sentry
{
	bool LuaServiceMgrComponent::Awake()
	{
		

		SceneScriptComponent * scriptComponent = Scene::GetComponent<SceneScriptComponent>();
		SceneProtocolComponent * protoComponent = Scene::GetComponent<SceneProtocolComponent>();

		string servicePath;
		SayNoAssertRetFalse_F(protoComponent);
		SayNoAssertRetFalse_F(scriptComponent);
		SayNoAssertRetFalse_F(App::Get().GetConfig().GetValue("Script", "service", servicePath));

		lua_State * lua = scriptComponent->GetLuaEnv();

		std::vector<std::string> servicePaths;
		SayNoAssertRetFalse_F(DirectoryHelper::GetFilePaths(servicePath, servicePaths));

		
		for (std::string & path : servicePaths)
		{
			if (!scriptComponent->LoadLuaScript(path))
			{
				SayNoDebugFatal("load " << path << " failure");
				return false;
			}
		}


		std::vector<std::string> services;
		protoComponent->GetServices(services);

		GameObject & serviceObject = App::Get().Service;
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

			ServiceBase * localService = serviceObject.GetComponent<ServiceBase>(service);
			if (localService == nullptr)
			{
				LuaServiceProxy * luaSerivce = new LuaServiceProxy();
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
			}
		}
		return true;
	}
}