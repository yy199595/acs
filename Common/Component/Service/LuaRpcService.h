#pragma once


#include<Script/LuaInclude.h>
#include"Service/RpcService.h"

class LuaTable;
namespace Sentry
{
    class LuaScriptComponent;

    class LuaRpcService : public RpcService, public IStart
    {
    public:
        LuaRpcService();
        ~LuaRpcService() override;
	public:
		bool InitService(const std::string & name, lua_State * luaEnv);
    public:
        bool Awake() final;
        void OnStart() final;
        bool LateAwake() final;
		const std::string &GetName()final { return this->mServiceName; }
    private:
		int mIdx;
		lua_State * mLuaEnv;
		std::string mServiceName;
        std::shared_ptr<LuaTable> mLuaTable;
        class RpcConfigComponent * mRpcConfigComponent;
        std::unordered_map<std::string, int> mMethodMap;
    };
}// namespace Sentry