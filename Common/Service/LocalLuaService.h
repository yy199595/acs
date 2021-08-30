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
		void Start() override { }
        bool HasMethod(const std::string &name) final;
		void GetMethods(std::vector<LocalActionProxy*> & methods) final;
		const std::string &GetServiceName()final { return this->GetTypeName(); } //TODO
        bool IsLuaService() final { return true; };

    protected:
        virtual XCode InvokeMethod(PacketMapper *) final;      
    private:
        int mServiceIndex;
        lua_State *mLuaEnv;
        SceneScriptComponent *mScriptManager;
        std::set<std::string> mMethodCacheSet;
        //std::unordered_map<std::string, int> mDefultActionMap;	//默认函数
        //std::unordered_map<std::string, NetLuaAction *> mActionMap;
    };
}// namespace Sentry