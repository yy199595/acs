#include"LocalLuaService.h"
#include"Script/Table.h"
#include"Script/Function.h"
#include"Method/LuaServiceMethod.h"
#include"Component/Lua/LuaScriptComponent.h"
#include"Global/ServiceConfig.h"
#include"Method/MethodRegister.h"
namespace Sentry
{
	LocalLuaService::LocalLuaService()
		: mLuaEnv(nullptr)
	{

	}

	LocalLuaService::~LocalLuaService()
	{
		//luaL_unref(this->mLuaEnv, LUA_REGISTRYINDEX, this->mIdx);
	}

	bool LocalLuaService::StartService()
	{
		this->mMethodRegister = std::make_shared<ServiceMethodRegister>(this);
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
			if(!this->mMethodRegister->AddMethod(std::make_shared<LuaServiceMethod>(config->Service, config->Method, this->mLuaEnv)))
			{
				return false;
			}
		}
		return true;
	}

	XCode LocalLuaService::Invoke(const std::string& name, std::shared_ptr<com::Rpc::Request> request,
		std::shared_ptr<com::Rpc::Response> response)
	{
		if(!this->IsStartService())
		{
			LOG_ERROR(this->GetName() << " is not start");
			return XCode::CallServiceNotFound;
		}
		response->set_rpc_id(request->rpc_id());
		response->set_user_id(request->user_id());

		std::shared_ptr<ServiceMethod> serviceMethod = this->mMethodRegister->GetMethod(name);
		if (serviceMethod == nullptr)
		{
			response->set_code((int)XCode::CallServiceNotFound);
			LOG_ERROR("not find lua [" << this->GetName() << "." << name << "]");
			return XCode::CallServiceNotFound;
		}

		try
		{
			return serviceMethod->Invoke(*request, *response);
		}
		catch (std::logic_error& logic_error)
		{
			response->set_error_str(logic_error.what());
			return XCode::ThrowError;
		}
	}


	bool LocalLuaService::LateAwake()
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

	bool LocalLuaService::OnStart()
	{
		const char * tab = this->GetName().c_str();
		WaitLuaTaskSource * luaTaskSource = Lua::Function::Call(this->mLuaEnv, tab, "OnStart");
		return(luaTaskSource == nullptr || luaTaskSource->Await<bool>());
	}
	bool LocalLuaService::CloseService()
	{
		return false;
	}

}