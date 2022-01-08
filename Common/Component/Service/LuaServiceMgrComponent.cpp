#include"LuaServiceMgrComponent.h"
#include"Core/App.h"
#include"Method/LuaServiceMethod.h"
#include"Component/ServiceBase/LuaServiceComponent.h"
#include"Scene/LuaScriptComponent.h"
#include"Scene/RpcConfigComponent.h"
#include"Util/DirectoryHelper.h"
namespace GameKeeper
{

    bool LuaServiceMgrComponent::Awake()
    {

        return true;
    }

    void LuaServiceMgrComponent::OnHotFix()
    {

    }

	bool LuaServiceMgrComponent::LateAwake()
	{
        LuaScriptComponent * luaComponent = nullptr;
        RpcConfigComponent * configComponent = nullptr;
        LOG_CHECK_RET_FALSE(luaComponent = this->GetComponent<LuaScriptComponent>());
        LOG_CHECK_RET_FALSE(configComponent = this->GetComponent<RpcConfigComponent>());


		lua_State * lua = luaComponent->GetLuaEnv();

		std::vector<std::string> services;
        configComponent->GetServices(services);

		for (std::string & service : services)
		{
			lua_getglobal(lua, service.c_str());
			if (!lua_istable(lua, -1))
			{
				continue;
			}
			std::vector<std::string> methods;
			if (!configComponent->GetMethods(service, methods))
			{
				continue;
			}
		
			auto localService = App::Get().GetComponent<ServiceComponent>(service);
			if (localService == nullptr)
			{
				auto luaSerivce = new LuaServiceComponent();
				if (App::Get().AddComponent(service, localService))
				{
					LOG_ERROR("add service " << service << " failure");
					return false;
				}
				if (!luaSerivce->Init(service) || !luaSerivce->InitService(service, lua))
				{
					LOG_FATAL("Init lua service [" << service << "] failure");
					return false;
				}
                luaSerivce->LateAwake();
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
                auto config = configComponent
                        ->GetProtocolConfig(service + "." + method);
				localService->AddMethod(new LuaServiceMethod(config, lua, idx));
				LOG_INFO("add new lua service method : " << service << "." << method);
			}
		}
		return true;
	}
}