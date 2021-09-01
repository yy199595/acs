#include "LocalService.h"

#include <Core/App.h>
#include <Scene/SceneActionComponent.h>
#include <Coroutine/CoroutineComponent.h>
#include <Scene/SceneProtocolComponent.h>
namespace Sentry
{
    LocalService::LocalService()
    {
    }

    bool LocalService::HasMethod(const std::string &method)
    {
		auto iter = this->mMethodMap.find(method);
		return iter != this->mMethodMap.end();
    }

	ServiceMethod * LocalService::GetMethod(const std::string &method)
	{
		auto iter = this->mMethodMap.find(method);
		return iter != this->mMethodMap.end() ? iter->second : nullptr;
	}

  //  bool LocalService::BindFunction(std::string name, LocalAction1 action)
  //  {
  //      return this->BindFunction(new LocalActionProxy1(name, action));
  //  }

  //  bool LocalService::BindFunction(LocalActionProxy * actionBox)
  //  {
		//SceneProtocolComponent * procolComponent = Scene::GetComponent<SceneProtocolComponent>();
		//const std::string & name = actionBox->GetName();
		//if (procolComponent->GetProtocolConfig(this->GetServiceName(), name) == nullptr)
		//{
		//	SayNoDebugFatal(this->GetServiceName() << "." << name << " not config");
		//	return false;
		//}
		//
  //      auto iter = this->mActionMap.find(name);
  //      if (iter != this->mActionMap.end())
  //      {
  //          SayNoDebugError("register " << this->GetTypeName() << "." << name << " fail");
  //          return false;
  //      }
  //      this->mActionMap.emplace(name, actionBox);
  //      return true;
  //  }

	bool LocalService::Bind(ServiceMethod * method)
	{
		SceneProtocolComponent * procolComponent = Scene::GetComponent<SceneProtocolComponent>();
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
		auto iter = this->mMethodMap.find(name);
		if (iter != this->mMethodMap.end())
		{
			SayNoDebugFatal(this->GetServiceName() << "." << name << " add failure");
			return false;
		}
		this->mMethodMap.emplace(name, method);
		return true;
	}
}