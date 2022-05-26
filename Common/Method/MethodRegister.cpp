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
		const std::string & name = method->GetName();
		const ServiceConfig & rpcConfig = App::Get()->GetServiceConfig();
		if (!rpcConfig.HasServiceMethod(this->mComponent->GetName(), name))
		{
			LOG_FATAL(this->mComponent->GetName() << "." << name << " add failure");
			return false;
		}

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
		App::Get()->GetServiceConfig().GetMethods(service, methods);
		for(const std::string & name : methods)
		{
			const char * tab = this->mComponent->GetName().c_str();
			if (Lua::Function::Get(lua, tab, name.c_str()))
			{
				std::shared_ptr<LuaServiceMethod> luaServiceMethod
						= std::make_shared<LuaServiceMethod>(service, name, lua);
				this->mLuaMethodMap.emplace(name, luaServiceMethod);
			}
			else if(this->GetMethod(name) == nullptr)
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