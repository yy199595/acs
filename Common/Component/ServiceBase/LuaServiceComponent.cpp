#include"LuaServiceComponent.h"
#include<Method/LuaServiceMethod.h>
#include<Scene/RpcConfigComponent.h>
#include"Script/LuaTable.h"
namespace GameKeeper
{
    LuaServiceComponent::LuaServiceComponent()
		: mIdx(0), mLuaEnv(nullptr)
    {

    }

    LuaServiceComponent::~LuaServiceComponent()
    {
        luaL_unref(this->mLuaEnv, LUA_REGISTRYINDEX, this->mIdx);
    }

	bool LuaServiceComponent::InitService(const std::string & name, lua_State * luaEnv)
	{
        this->mLuaEnv = luaEnv;
		this->mServiceName = name;
        this->mLuaTable = LuaTable::Create(luaEnv, name);
        return this->mLuaTable != nullptr;
	}

    bool LuaServiceComponent::Awake()
    {
        auto luaFunction = this->mLuaTable->GetFunction("Awake");
        return luaFunction == nullptr || luaFunction->Func<bool>();
    }

    bool LuaServiceComponent::LateAwake()
    {
        auto luaFunction = this->mLuaTable->GetFunction("LateAwake");
        return luaFunction == nullptr || luaFunction->Func<bool>();
    }

    void LuaServiceComponent::OnStart()
    {
        lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->mIdx);
        lua_getfield(this->mLuaEnv, -1, "OnStart");
        if (lua_isfunction(this->mLuaEnv, -1))
        {
            return;
        }
        lua_State *coroutine = lua_newthread(this->mLuaEnv);
        lua_pushvalue(this->mLuaEnv, -2);
        lua_xmove(this->mLuaEnv, coroutine, 1);
        lua_resume(coroutine, this->mLuaEnv, 0);
    }
}