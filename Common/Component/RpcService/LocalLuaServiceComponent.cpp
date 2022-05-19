#include"LocalLuaServiceComponent.h"
#include"Script/Table.h"
#include"Script/Function.h"
#include"Method/LuaServiceMethod.h"
#include"Component/Lua/LuaScriptComponent.h"
#include"Global/ServiceConfig.h"

namespace Sentry
{
	LocalLuaServiceComponent::LocalLuaServiceComponent()
		: mLuaEnv(nullptr)
	{

	}

	LocalLuaServiceComponent::~LocalLuaServiceComponent()
	{
		//luaL_unref(this->mLuaEnv, LUA_REGISTRYINDEX, this->mIdx);
	}

	bool LocalLuaServiceComponent::OnStartService(ServiceMethodRegister & methodRegister)
	{
		std::vector<std::string> methods;
		const char * tab = this->GetName().c_str();
		const ServiceConfig & rpcConfig = this->GetApp()->GetServiceConfig();
		rpcConfig.GetMethods(this->GetName(), methods);

		for (const std::string& method : methods)
		{
			const char * func = method.c_str();
			if(!Lua::Function::Get(this->mLuaEnv, tab, func))
			{
				LOG_ERROR("not find rpc method = [" << tab <<'.' << func << ']');
				return false;
			}
			string fullName = fmt::format("{0}.{1}", this->GetName(), method);
			const InterfaceConfig * config = rpcConfig.GetInterfaceConfig(fullName);
			if(!methodRegister.AddMethod(std::make_shared<LuaServiceMethod>(config->Service, config->Method, this->mLuaEnv)))
			{
				return false;
			}
		}
		return true;
	}


	bool LocalLuaServiceComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(ServiceComponent::LateAwake());
		this->mLuaComponent = this->GetComponent<LuaScriptComponent>();
		LOG_CHECK_RET_FALSE(this->mLuaEnv = this->mLuaComponent->GetLuaEnv());
		std::shared_ptr<Lua::Table> luaTable = Lua::Table::Create(this->mLuaEnv, this->GetName());
		if (luaTable == nullptr)
		{
			LOG_ERROR(this->GetName() << " is not lua table");
			return false;
		}
		const char * tab = this->GetName().c_str();
		if(Lua::lua_getfunction(this->mLuaEnv, tab, "Awake"))
		{
			LOG_CHECK_RET_FALSE(Lua::Function::Invoke<bool>(this->mLuaEnv));
		}
		if(Lua::lua_getfunction(this->mLuaEnv, tab, "LateAwake"))
		{
			LOG_CHECK_RET_FALSE(Lua::Function::Invoke<bool>(this->mLuaEnv));
		}
		return true;
	}

	bool LocalLuaServiceComponent::OnStart()
	{
		const char * tab = this->GetName().c_str();
		LuaTaskSource * luaTaskSource = Lua::Function::Call(this->mLuaEnv, tab, "OnStart");
		return(luaTaskSource == nullptr || luaTaskSource->Await<bool>());
	}

}