//
// Created by zmhy0073 on 2022/6/6.
//

#include"LocalLuaHttpService.h"
#include"Lua/Function.h"
#include"Component/LuaScriptComponent.h"
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
        this->mLuaEnv = luaComponent->GetLuaEnv();
        const char * tab = this->GetName().c_str();
        if(Lua::lua_getfunction(this->mLuaEnv, tab, "Awake"))
        {
            if(lua_pcall(this->mLuaEnv, 0, 0, 0) != LUA_OK)
            {
                LOG_ERROR(lua_tostring(this->mLuaEnv, -1));
                return false;
            }
        }
        return true;
    }

    bool LocalLuaHttpService::OnCloseService()
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

    bool LocalLuaHttpService::OnStartService(HttpServiceRegister &serviceRegister)
    {
        std::vector<const HttpMethodConfig *> httpMethodConfigs;
        LuaScriptComponent * luaComponent = this->GetComponent<LuaScriptComponent>();
        const HttpServiceConfig * httpServiceConfig = HttpConfig::Inst()->GetConfig(this->GetName());

        httpServiceConfig->GetMethodConfigs(httpMethodConfigs);
        for(const HttpMethodConfig * methodConfig : httpMethodConfigs)
        {
            const char * tab = methodConfig->Service.c_str();
            const char * method = methodConfig->Method.c_str();
            if(!Lua::Function::Get(luaComponent->GetLuaEnv(), tab, method))
            {
                return false;
            }
        }

        const char * t = this->GetName().c_str();
        if(Lua::lua_getfunction(this->mLuaEnv, t, "OnServiceStart"))
        {
            if(lua_pcall(this->mLuaEnv, 0, 0, 0) != LUA_OK)
            {
                LOG_ERROR(lua_tostring(this->mLuaEnv, -1));
                return false;
            }
        }
        return true;
    }
}