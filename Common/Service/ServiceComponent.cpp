#include"ServiceComponent.h"
#include<Core/App.h>
#include <Method/LuaServiceMethod.h>
#include<Scene/ProtocolComponent.h>
namespace Sentry
{
	bool ServiceComponent::AddMethod(ServiceMethod * method)
	{
		ProtocolComponent * procolComponent = App::Get().GetComponent<ProtocolComponent>();
		if (procolComponent == nullptr)
		{
			return false;
		}
		const std::string & name = method->GetName();
		if (procolComponent->GetProtocolConfig(this->GetServiceName(), name) == nullptr)
		{
			SayNoDebugFatal(this->GetServiceName() << "." << name << " not config");
			return false;
		}
		if (method->IsLuaMethod())
		{
			auto iter = this->mLuaMethodMap.find(name);
			if (iter != this->mLuaMethodMap.end())
			{
				SayNoDebugFatal(this->GetServiceName() << "." << name << " add failure");
				return false;
			}
			this->mLuaMethodMap.emplace(name, method);
		}
		else
		{
			auto iter = this->mMethodMap.find(name);
			if (iter != this->mMethodMap.end())
			{
				SayNoDebugFatal(this->GetServiceName() << "." << name << " add failure");
				return false;
			}
			this->mMethodMap.emplace(name, method);
		}
		return true;
	}
	bool ServiceComponent::HasMethod(const std::string & method)
	{
		auto iter = this->mLuaMethodMap.find(method);
		if (iter != this->mLuaMethodMap.end())
		{
			return true;
		}
		auto iter1 = this->mMethodMap.find(method);
		return iter1 != this->mMethodMap.end();
	}
	ServiceMethod * ServiceComponent::GetMethod(const std::string & method)
	{
		auto iter = this->mLuaMethodMap.find(method);
		if (iter != this->mLuaMethodMap.end())
		{
			return iter->second;
		}
		auto iter1 = this->mMethodMap.find(method);
		return iter1 != this->mMethodMap.end() ? iter1->second : nullptr;
	}
}
