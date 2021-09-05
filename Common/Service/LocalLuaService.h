#pragma once

#include "ServiceBase.h"
#include <Script/LuaInclude.h>

namespace Sentry
{
    class SceneScriptComponent;

    class LocalLuaService : public ServiceBase
    {
    public:
        LocalLuaService(lua_State *luaEnv, int index);

        ~LocalLuaService();

    public:
        bool Awake() final;
		bool IsLuaService() final { return true; };
		const std::string &GetServiceName()final { return this->GetTypeName(); } //TODO
        
    private:
        int mServiceIndex;
        lua_State *mLuaEnv;
        SceneScriptComponent *mScriptManager;
        std::set<std::string> mMethodCacheSet;
        //std::unordered_map<std::string, int> mDefultActionMap;	//默认函数
        //std::unordered_map<std::string, NetLuaAction *> mActionMap;
    };
}// namespace Sentry