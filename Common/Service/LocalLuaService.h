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

        bool HasMethod(const std::string &name) final;

        bool IsLuaService() final { return true; };

    protected:
        virtual XCode InvokeMethod(NetMessageProxy *) final;      
    private:
        int mServiceIndex;
        lua_State *mLuaEnv;
        SceneScriptComponent *mScriptManager;
        std::set<std::string> mMethodCacheSet;
        //std::unordered_map<std::string, int> mDefultActionMap;	//默认函数
        //std::unordered_map<std::string, NetLuaAction *> mActionMap;
    };
}// namespace Sentry