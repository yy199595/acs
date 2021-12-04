#include"Service/ServiceComponent.h"
#include<Core/App.h>
#include <Method/LuaServiceMethod.h>
#include"Method/JsonServiceMethod.h"
#include<Scene/RpcConfigComponent.h>
namespace GameKeeper
{
	bool ServiceComponent::AddMethod(ServiceMethod * method)
    {
        auto *procolComponent = App::Get().GetComponent<RpcConfigComponent>();
        if (procolComponent == nullptr)
        {
            return false;
        }
        const std::string &name = method->GetName();
        const std::string &service = this->GetServiceName();

        if (!procolComponent->HasServiceMethod(service, name))
        {
            LOG_FATAL(this->GetServiceName() << "." << name << " not config");
            return false;
        }

        auto iter = this->mMethodMap.find(name);
        if (iter != this->mMethodMap.end())
        {
            LOG_FATAL(this->GetServiceName() << "." << name << " add failure");
            return false;
        }
        this->mMethodMap.emplace(name, method);
        return true;
    }
	bool ServiceComponent::HasProtoMethod(const std::string & method)
	{
		auto iter1 = this->mMethodMap.find(method);
		return iter1 != this->mMethodMap.end();
	}
	ServiceMethod * ServiceComponent::GetProtoMethod(const std::string & method)
	{
		auto iter1 = this->mMethodMap.find(method);
		return iter1 != this->mMethodMap.end() ? iter1->second : nullptr;
	}

    bool ServiceComponent::AddMethod(JsonServiceMethod *method)
    {
        auto *procolComponent = App::Get().GetComponent<RpcConfigComponent>();
        if (procolComponent == nullptr)
        {
            return false;
        }
        const std::string &name = method->GetName();
        const std::string &service = this->GetServiceName();

        if (!procolComponent->HasServiceMethod(service, name))
        {
            LOG_FATAL(this->GetServiceName() << "." << name << " not config");
            return false;
        }
        this->mJsonMethodMap.emplace(name, method);
        return true;
    }

    bool ServiceComponent::HasJsonMethod(const std::string &method)
    {
        auto iter = this->mJsonMethodMap.find(method);
        return iter != this->mJsonMethodMap.end();
    }

    JsonServiceMethod *ServiceComponent::GetJsonMethod(const std::string &method)
    {
        auto iter = this->mJsonMethodMap.find(method);
        return iter != this->mJsonMethodMap.end() ? iter->second : nullptr;
    }
}
