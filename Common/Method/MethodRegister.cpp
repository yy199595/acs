//
// Created by mac on 2022/4/12.
//

#include"MethodRegister.h"
#include"LuaServiceMethod.h"
#include"Script/Function.h"
#include"App/App.h"
#include"Component/RpcService/LocalServiceComponent.h"
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

	ServiceMethodRegister::ServiceMethodRegister(Component * component)
			: mComponent(component)
	{

	}
}

namespace Sentry
{
	std::shared_ptr<HttpServiceMethod> HttpServiceRegister::GetMethod(const string& name)
	{
        auto iter = this->mLuaHttpMethodMap.find(name);
        if(iter != this->mLuaHttpMethodMap.end())
        {
            return iter->second;
        }
		auto iter1 = this->mHttpMethodMap.find(name);
		return iter1 != this->mHttpMethodMap.end() ? iter1->second : nullptr;
	}

    bool HttpServiceRegister::AddMethod(std::shared_ptr<HttpServiceMethod> method)
    {
        const std::string & name = method->GetName();
        if(method->IsLuaMethod())
        {
            auto iter = this->mLuaHttpMethodMap.find(name);
            if(iter != this->mLuaHttpMethodMap.end())
            {
                return false;
            }
            this->mLuaHttpMethodMap.emplace(name, method);
            return true;
        }
        auto iter1 = this->mHttpMethodMap.find(name);
        if(iter1 != this->mHttpMethodMap.end())
        {
            return false;
        }
        this->mHttpMethodMap.emplace(name, method);
        return true;
    }
}