//
// Created by mac on 2022/4/12.
//

#include"MethodRegister.h"
#include"LuaServiceMethod.h"
#include"Script/Function.h"
#include"App/App.h"
namespace Sentry
{
	bool ServiceMethodRegister::AddMethod(std::shared_ptr<ServiceMethod> method)
	{
		ServiceComponent * serviceComponent = this->mComponent->Cast<ServiceComponent>();
		if(serviceComponent == nullptr)
		{
			return false;
		}
		const RpcServiceConfig & rpcServiceConfig = serviceComponent->GetServiceConfig();
		if(!rpcServiceConfig.GetConfig(method->GetName()))
		{
			return false;
		}
		const std::string & name = method->GetName();
		if (method->IsLuaMethod())
		{
			auto iter = this->mLuaMethodMap.find(name);
			if (iter != this->mLuaMethodMap.end())
			{
				this->mLuaMethodMap.erase(iter);
			}
			this->mLuaMethodMap.emplace(name, method);
			//LOG_DEBUG("add new lua service method [" << this->mService << '.' << name << "]");
			return true;
		}

		auto iter = this->mMethodMap.find(name);
		if (iter != this->mMethodMap.end())
		{
			LOG_FATAL(this->mComponent->GetName() << "." << name << " add failure");
			return false;
		}
		this->mMethodMap.emplace(name, method);
		//LOG_DEBUG("add new c++ service method [" << this->mService <<'.' << name << ']');
		return true;
	}

	std::shared_ptr<ServiceMethod> ServiceMethodRegister::GetMethod(const string& name)
	{
		auto iter = this->mLuaMethodMap.find(name);
		if(iter != this->mLuaMethodMap.end())
		{
			return iter->second;
		}
		auto iter1 = this->mMethodMap.find(name);
		if(iter1 != this->mMethodMap.end())
		{
			return iter1->second;
		}
		return nullptr;
	}

	bool ServiceMethodRegister::LoadLuaMethod(lua_State* lua)
	{
		std::vector<std::string> methods;
		const std::string & service = this->mComponent->GetName();
		ServiceComponent * serviceComponent = this->mComponent->Cast<ServiceComponent>();
		if(serviceComponent == nullptr)
		{
			return false;
		}
		std::vector<const RpcInterfaceConfig *> rpcInterfaceConfigs;
		const RpcServiceConfig & rpcServiceConfig = serviceComponent->GetServiceConfig();
		rpcServiceConfig.GetConfigs(rpcInterfaceConfigs);

		for(const RpcInterfaceConfig * rpcInterfaceConfig : rpcInterfaceConfigs)
		{
			const char* tab = rpcInterfaceConfig->Service.c_str();
			const char * func = rpcInterfaceConfig->Method.c_str();
			if (Lua::Function::Get(lua, tab, func))
			{
				std::shared_ptr<LuaServiceMethod> luaServiceMethod
					= std::make_shared<LuaServiceMethod>(service, rpcInterfaceConfig->Method, lua);
				this->mLuaMethodMap.emplace(rpcInterfaceConfig->Method, luaServiceMethod);
			}
			else if (this->GetMethod(rpcInterfaceConfig->Method) == nullptr)
			{
				return false;
			}
		}
		return true;
	}

	ServiceMethodRegister::ServiceMethodRegister(Component * component)
			: mComponent(component)
	{

	}
}

namespace Sentry
{
	std::shared_ptr<HttpServiceMethod> HttpServiceRegister::GetMethod(const string& name)
	{
		auto iter = this->mHttpMethodMap.find(name);
		return iter != this->mHttpMethodMap.end() ? iter->second : nullptr;
	}
}