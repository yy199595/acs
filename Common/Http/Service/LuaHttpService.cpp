//
// Created by zmhy0073 on 2022/6/6.
//

#include"LuaHttpService.h"
#include"Lua/Function.h"
#include"Method/LuaHttpServiceMethod.h"
#include"Component/LuaScriptComponent.h"
namespace Sentry
{
    bool LuaHttpService::LateAwake()
    {
        this->mLuaComponent = this->GetComponent<LuaScriptComponent>();
        Lua::LuaModule * luaModule = this->mLuaComponent->LoadModule(this->GetName());
        if(luaModule == nullptr)
        {
            LOG_ERROR("load http module error : " << this->GetName());
            return false;
        }
        return luaModule->Awake();
    }

    bool LuaHttpService::OnCloseService()
    {
        Lua::LuaModule* luaModule = this->mLuaComponent->GetModule(this->GetName());
        return luaModule != nullptr && luaModule->Close();
    }

    bool LuaHttpService::OnStartService(HttpServiceRegister &serviceRegister)
    {
        return true;
    }
}