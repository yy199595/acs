#include"LuaService.h"
#include"Lua/Table.h"
#include"Lua/Function.h"
#include"Lua/LuaServiceMethod.h"
#include"Component/LuaScriptComponent.h"
#include"Config/ServiceConfig.h"
#include"Method/MethodRegister.h"
namespace Sentry
{
	LuaService::LuaService()
		: mLuaEnv(nullptr)
	{

	}

	LuaService::~LuaService()
	{
		//luaL_unref(this->mLuaEnv, LUA_REGISTRYINDEX, this->mIdx);
	}

	bool LuaService::StartNewService()
	{
		this->mMethodRegister = std::make_shared<ServiceMethodRegister>(this);

		std::vector<const RpcInterfaceConfig *> rpcInterConfigs;
		const RpcServiceConfig & rpcServiceConfig = this->GetServiceConfig();
		rpcServiceConfig.GetConfigs(rpcInterConfigs);

		for(const RpcInterfaceConfig * rpcInterfaceConfig : rpcInterConfigs)
		{
			const char* func = rpcInterfaceConfig->Method.c_str();
			const char * tab = rpcInterfaceConfig->Service.c_str();
			if (!Lua::Function::Get(this->mLuaEnv, tab, func))
			{
				LOG_ERROR("not find rpc method = [" << tab << '.' << func << ']');
				return false;
			}
			if (!this->mMethodRegister->AddMethod(std::make_shared<LuaServiceMethod>
			        (rpcInterfaceConfig, this->mLuaEnv)))
			{
				return false;
			}
		}
        const char * tab = this->GetName().c_str();
        if(Lua::Table::Get(this->mLuaEnv, this->GetName()))
        {
            lua_pushboolean(this->mLuaEnv, true);
            lua_setfield(this->mLuaEnv, -2, "IsStartService");
        }
        const char * t = this->GetName().c_str();
        if(Lua::lua_getfunction(this->mLuaEnv, t, "OnServiceStart"))
        {
            if(lua_pcall(this->mLuaEnv, 0, 0, 0) != LUA_OK)
            {
                LOG_ERROR(lua_tostring(this->mLuaEnv, -1));
                return false;
            }
        }
        this->AddHost(this->GetConfig().GetLocalHost());
        return true;
	}

	XCode LuaService::Invoke(const std::string &name, std::shared_ptr<Rpc::Data> message)
	{
		if(!this->IsStartService())
		{
			LOG_ERROR(this->GetName() << " is not start");
			return XCode::CallServiceNotFound;
		}
		std::shared_ptr<ServiceMethod> serviceMethod = this->mMethodRegister->GetMethod(name);
		if (serviceMethod == nullptr)
		{
			return XCode::CallServiceNotFound;
		}
		return serviceMethod->Invoke(*message);
	}

	bool LuaService::LateAwake()
	{
		LOG_CHECK_RET_FALSE(Service::LateAwake());
		this->mLuaComponent = this->GetComponent<LuaScriptComponent>();
		LOG_CHECK_RET_FALSE(this->mLuaEnv = this->mLuaComponent->GetLuaEnv());

        if(!Lua::Table::Get(this->mLuaEnv, this->GetName()))
        {
            LOG_ERROR(this->GetName() << " is not lua table");
            return false;
        }
        lua_pushboolean(this->mLuaEnv, false);
        lua_setfield(this->mLuaEnv, -2, "IsStartService");
		return true;
	}

	bool LuaService::CloseService()
	{
        const char * tab = this->GetName().c_str();
        if(Lua::lua_getfunction(this->mLuaEnv, tab, "OnServiceStart"))
        {
            if(lua_pcall(this->mLuaEnv, 0, 0, 0) != LUA_OK)
            {
                LOG_ERROR(lua_tostring(this->mLuaEnv, -1));
                return false;
            }
        }
        return true;
	}

}