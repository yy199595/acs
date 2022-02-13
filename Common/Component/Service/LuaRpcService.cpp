#include"LuaRpcService.h"
#include<Method/LuaServiceMethod.h>
#include<Scene/RpcConfigComponent.h>
#include"Script/LuaTable.h"
#include"Scene/LuaScriptComponent.h"
namespace Sentry
{
    LuaRpcService::LuaRpcService()
		: mLuaEnv(nullptr)
    {

    }

    LuaRpcService::~LuaRpcService()
    {
        //luaL_unref(this->mLuaEnv, LUA_REGISTRYINDEX, this->mIdx);
    }

    bool LuaRpcService::Awake()
    {
        return true;
    }

    void LuaRpcService::OnHotFix()
    {

    }

    bool LuaRpcService::LateAwake()
    {
        this->mLuaComponent = this->GetComponent<LuaScriptComponent>();
        this->mConfigComponent = this->GetComponent<RpcConfigComponent>();
        LOG_CHECK_RET_FALSE(this->mLuaComponent && this->mConfigComponent);
        LOG_CHECK_RET_FALSE(this->mLuaEnv = this->mLuaComponent->GetLuaEnv());

        lua_getglobal(this->mLuaEnv, this->GetName().c_str());
        LOG_CHECK_RET_FALSE(lua_istable(this->mLuaEnv, -1));

        int ref = luaL_ref(this->mLuaEnv, LUA_REGISTRYINDEX);
        LuaTable luaTable(this->mLuaEnv, ref, this->GetName());
        std::shared_ptr<LuaFunction> awakeFunction = luaTable.GetFunction("Awake");
        std::shared_ptr<LuaFunction> lateAwakeFunction = luaTable.GetFunction("LateAwake");

        if(awakeFunction != nullptr)
        {
            LOG_CHECK_RET_FALSE(awakeFunction->Func<bool>());
        }
        if(lateAwakeFunction != nullptr)
        {
            LOG_CHECK_RET_FALSE(lateAwakeFunction->Func<bool>());
        }

        std::vector<std::string> methods;
        this->mConfigComponent->GetMethods(this->GetName(), methods);

        for(const std::string & method : methods)
        {
            lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, ref);
            lua_getfield(this->mLuaEnv, -1, method.c_str());
            LOG_CHECK_RET_FALSE(lua_isfunction(this->mLuaEnv, -1));
            int idx = luaL_ref(this->mLuaEnv, LUA_REGISTRYINDEX);
            auto config = this->mConfigComponent
                    ->GetProtocolConfig(this->GetName() + "." + method);
            LOG_INFO("add new lua service method : ", this->GetName(), '.', method);
            this->AddMethod(std::make_shared<LuaServiceMethod>(config, this->mLuaEnv, idx));
        }
        return true;
    }

    void LuaRpcService::OnStart()
    {
        lua_getglobal(this->mLuaEnv, this->GetName().c_str());
        lua_getfield(this->mLuaEnv, -1, "OnStart");
        if (!lua_isfunction(this->mLuaEnv, -1))
        {
            return;
        }
        int ref = lua_ref(this->mLuaEnv);
        this->mLuaComponent->Invoke(ref)->Await();
        luaL_unref(this->mLuaEnv, LUA_REGISTRYINDEX, ref);
        LOG_WARN("Invoke ", this->GetName(), " Start Coroutine")
    }
}