﻿#pragma once

#include "ServiceComponent.h"
#include <Script/LuaInclude.h>

namespace Sentry
{
    class LuaScriptComponent;

    class LuaServiceComponent : public ServiceComponent
    {
    public:
        LuaServiceComponent();

        ~LuaServiceComponent();
	public:
		bool InitService(const std::string name, lua_State * luaEnv);

    public:
        bool Awake() final;
		void Start() final;
		bool IsLuaService() final { return true; };
		const std::string &GetServiceName()final { return this->GetTypeName(); } //TODO
        
    private:
		int mIdx;
		lua_State * mLuaEnv;
		std::string mServiceName;
    };
}// namespace Sentry