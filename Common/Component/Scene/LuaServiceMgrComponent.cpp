#include"LuaServiceMgrComponent.h"
#include"Object/App.h"
#include"Method/LuaServiceMethod.h"
#include"Service/LuaRpcService.h"
#include"Scene/LuaScriptComponent.h"
#include"Scene/RpcConfigComponent.h"
#include"Util/DirectoryHelper.h"
namespace Sentry
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
        LOG_THROW_ERROR(this->mLuaComponent = this->GetComponent<LuaScriptComponent>());
        LOG_THROW_ERROR(this->mConfigComponent = this->GetComponent<RpcConfigComponent>());

		std::vector<std::string> services;
        const ServerConfig & config = App::Get().GetConfig();
        LOG_THROW_ERROR(config.GetValue("service", services));

		for (std::string & service : services)
		{
            auto luaService = this->GetComponent<RpcService>(service);
            if(luaService != nullptr && !this->AddMethod(luaService))
            {
                LOG_ERROR(luaService->GetName(), " add lua method failure");
                return false;
            }
		}
        return true;
	}
}

    bool LuaServiceMgrComponent::AddMethod(RpcService *rpcService)
    {
        std::vector<std::string> methods;
        if (!this->mConfigComponent->GetMethods(this->GetName(), methods))
        {
            return true;
        }
        lua_State * lua = this->mLuaComponent->GetLuaEnv();
        int ref = luaL_ref(lua, LUA_REGISTRYINDEX);
        for (std::string & method : methods)
        {
            lua_rawgeti(lua, LUA_REGISTRYINDEX, ref);
            lua_getfield(lua, -1, method.c_str());
            if (!lua_isfunction(lua, -1))
            {
                return false;
            }
            int idx = luaL_ref(lua, LUA_REGISTRYINDEX);
            auto config = this->mConfigComponent
                    ->GetProtocolConfig(this->GetName() + "." + method);
            LOG_INFO("add new lua service method : ", this->GetName(), '.', method);
            rpcService->AddMethod(std::make_shared<LuaServiceMethod>(config, lua, idx));
        }
        return true;
    }
