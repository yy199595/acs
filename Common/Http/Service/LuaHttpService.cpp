//
// Created by zmhy0073 on 2022/6/6.
//

#include"LuaHttpService.h"
#include"Lua/Function.h"
#include"Module/LuaModule.h"
#include"Component/LuaScriptComponent.h"
namespace Sentry
{
	LuaHttpService::LuaHttpService()
	{
		this->mLuaComponent = nullptr;
	}

    bool LuaHttpService::OnInit()
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
}