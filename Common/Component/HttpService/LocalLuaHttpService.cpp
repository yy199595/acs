//
// Created by zmhy0073 on 2022/6/6.
//

#include"LocalLuaHttpService.h"
#include"Script/Function.h"
#include"Component/Lua/LuaScriptComponent.h"
namespace Sentry
{

    bool LocalLuaHttpService::LateAwake()
    {
        LuaScriptComponent * luaComponent = this->GetComponent<LuaScriptComponent>();
        if(!Lua::Table::Get(luaComponent->GetLuaEnv(), this->GetName()))
        {
            LOG_ERROR(this->GetName() << " is not lua table");
            return false;
        }

        const char * tab = this->GetName().c_str();
        if(Lua::lua_getfunction(luaComponent->GetLuaEnv(), tab, "Awake"))
        {
            LOG_CHECK_RET_FALSE(Lua::Function::Invoke<bool>(luaComponent->GetLuaEnv()));
        }
        return true;
    }

    bool LocalLuaHttpService::OnStartService(HttpServiceRegister &serviceRegister)
    {
        LuaScriptComponent * luaComponent = this->GetComponent<LuaScriptComponent>();
        std::vector<const HttpInterfaceConfig *> httpConfigs;
        this->GetServiceConfig().GetConfigs(httpConfigs);
        for(const HttpInterfaceConfig * config : httpConfigs)
        {
            const char * tab = config->Service.c_str();
            const char * method = config->Method.c_str();
            if(!Lua::Function::Get(luaComponent->GetLuaEnv(), tab, method))
            {
                return false;
            }
        }
        return true;
    }
}