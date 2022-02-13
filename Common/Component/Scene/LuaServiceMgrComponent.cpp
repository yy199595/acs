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
        const ServerConfig &config = App::Get().GetConfig();
        LOG_THROW_ERROR(config.GetValue("service", services));

        for (std::string &service: services) {
            auto luaService = this->GetComponent<RpcService>(service);
            if (luaService != nullptr && !this->AddMethod(luaService)) {
                LOG_ERROR(luaService->GetName(), " add lua method failure");
                return false;
            }
        }
        return true;
    }


    bool LuaServiceMgrComponent::AddMethod(RpcService *rpcService)
    {
        std::vector<std::string> methods;
        const std::string &name = rpcService->GetName();
        if (!this->mConfigComponent->GetMethods(name, methods)) {
            return true;
        }
        lua_State *lua = this->mLuaComponent->GetLuaEnv();
        for (std::string &method: methods)
        {
            if (!lua_getfunction(lua, name.c_str(), method.c_str())) {
                continue;
            }
            int idx = luaL_ref(lua, LUA_REGISTRYINDEX);
            auto config = this->mConfigComponent
                    ->GetProtocolConfig( name + "." + method);
            if(!rpcService->AddMethod(std::make_shared<LuaServiceMethod>(config, lua, idx)))
            {
                return false;
            }
        }
        return true;
    }
}
