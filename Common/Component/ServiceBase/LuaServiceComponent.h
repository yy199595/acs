#pragma once

#include "Component/ServiceBase/ServiceComponent.h"
#include <Script/LuaInclude.h>

namespace GameKeeper
{
    class LuaScriptComponent;

    class LuaServiceComponent : public ServiceComponent, public IStart
    {
    public:
        LuaServiceComponent();
        ~LuaServiceComponent() override;
	public:
		bool InitService(const std::string & name, lua_State * luaEnv);
    public:
        bool Awake() final;
        void OnStart() final;
        bool LateAwake() final;
		const std::string &GetServiceName()final { return this->mServiceName; }
    private:
		int mIdx;
		lua_State * mLuaEnv;
		std::string mServiceName;
        class RpcConfigComponent * mRpcConfigComponent;
        std::unordered_map<std::string, int> mMethodMap;
    };
}// namespace GameKeeper