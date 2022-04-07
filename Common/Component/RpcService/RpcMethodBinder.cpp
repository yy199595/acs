//
// Created by mac on 2022/4/7.
//
#include"RpcMethodBinder.h"
#include"App/App.h"
#include"Global/RpcConfig.h"
namespace Sentry
{
	bool RpcMethodBinder::AddMethod(std::string service, std::shared_ptr<ServiceMethod> method)
	{
		const RpcConfig & rpcConfig = App::Get()->GetRpcConfig();
		const std::string& name = method->GetName();
		if (!rpcConfig.HasServiceMethod(service, name))
		{
			LOG_FATAL(service, '.', name, " add failure");
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
			LOG_DEBUG("add new lua service method [{0}.{1}]", service, name);
			return true;
		}

		auto iter = this->mMethodMap.find(name);
		if (iter != this->mMethodMap.end())
		{
			LOG_FATAL("{0}.{1} add failure", service, name);
			return false;
		}
		this->mMethodMap.emplace(name, method);
		LOG_DEBUG("add new c++ service method [{0}.{1}]", service, name);
		return true;
	}

	std::shared_ptr<ServiceMethod> RpcMethodBinder::GetMethod(const string& name)
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
}