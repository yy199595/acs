//
// Created by zmhy0073 on 2022/6/6.
//

#include"LuaHttpService.h"
#include"Lua/Function.h"
#include"Component/LuaScriptComponent.h"
namespace Sentry
{

    bool LuaHttpService::LateAwake()
    {
        LuaScriptComponent * luaComponent = this->GetComponent<LuaScriptComponent>();
        if(!luaComponent->LoadModule(this->GetName()))
        {
            LOG_ERROR("load http module error : " << this->GetName());
            return false;
        }
        return true;
    }

    bool LuaHttpService::OnCloseService()
    {
        const char * t = this->GetName().c_str();
        if(Lua::lua_getfunction(this->mLuaEnv, t, "OnServiceClose"))
        {
            if(lua_pcall(this->mLuaEnv, 0, 0, 0) != LUA_OK)
            {
                LOG_ERROR(lua_tostring(this->mLuaEnv, -1));
                return false;
            }
        }
        return true;
    }

    bool LuaHttpService::OnStartService(HttpServiceRegister &serviceRegister)
    {
        std::vector<const HttpMethodConfig *> httpMethodConfigs;
        LuaScriptComponent *luaComponent = this->GetComponent<LuaScriptComponent>();
        const HttpServiceConfig *httpServiceConfig = HttpConfig::Inst()->GetConfig(this->GetName());

        if (httpServiceConfig->GetMethodConfigs(httpMethodConfigs) <= 0)
        {
            return false;
        }
        for (const HttpMethodConfig *methodConfig: httpMethodConfigs)
        {
            const std::string &tab = methodConfig->Service;
            const std::string &func = methodConfig->Method;
            if (!luaComponent->GetFunction(tab, func))
            {
                return false;
            }
        }
        return true;
    }
}