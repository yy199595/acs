#pragma once

#include "ServiceBase.h"
#include <Script/LuaInclude.h>

namespace Sentry
{
    class SceneScriptComponent;

    class LuaServiceProxy : public ServiceBase
    {
    public:
        LuaServiceProxy();

        ~LuaServiceProxy();
	public:
		bool InitService(const std::string name, lua_State * luaEnv);

    public:
        bool Awake() final;
		bool IsLuaService() final { return true; };
		const std::string &GetServiceName()final { return this->GetTypeName(); } //TODO
        
    private:
		int mIdx;
		lua_State * mLuaEnv;
		std::string mServiceName;
    };
}// namespace Sentry